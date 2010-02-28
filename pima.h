



#ifndef __CARIOU_PIMA_H__
#define __CARIOU_PIMA_H__

#include "tools.h"

class Pima
{
     protected:
	  int W,H;
          CImg<unsigned char> srcFullRGB;
          CImg<float> srcFullCIE;
          CImg<float> srcToFillCIE;  
          CImg<unsigned char> srcToFillRGB;            
         
          //CImg<float> imageXYZ; 
          CImg<bool> target_region;
          CImg<bool> source_region;
          CImg<bool> fill_front;
          CImg<float> CP;//Confidence term
          CImg<float> DP;//Data term
          CImg<float> PP;//Priority values
          int size_dilate;
          int size_patch;
          int alpha;
	  
	 
     public:

          Pima() : srcFullRGB(),srcToFillRGB() {}
          ~Pima() {}

          Pima(const char * _srcFullRGB,const char * _srcToFillRGB, int _size_dilate=25, int _size_patch=9,int _alpha=255) : 
               srcFullRGB(), srcToFillRGB(), size_dilate(), size_patch(), alpha()
          {     
               //initialisation 
               
               CImg<unsigned char> tmp1(_srcFullRGB);
               CImg<unsigned char> tmp2(_srcToFillRGB);
	       W=tmp1.width(),H=tmp1.height();
               CImg<bool> tmp3(W,H,1,1);
               tmp3.fill(0);
               CImg<float> tmp4(W,H,1,1);
               tmp4.fill(0.0);
               CImg<bool> tmp5(W,H,1,1);
               tmp5.fill(0);
	       
	       target_region=tmp3;
	       source_region=tmp3;
	       fill_front=tmp3;
               
               
               srcFullRGB=tmp1;
               srcToFillRGB=tmp2;
               size_dilate=_size_dilate;
               size_patch=_size_patch;
               alpha=_alpha;
               
               srcToFillCIE=srcToFillRGB.get_RGBtoLab();

               //Initialisation of target_region
	       cimg_forXY(srcFullRGB,g,h){
			 //if (srcFullRGB(g,h)!=srcToFillRGB(g,h))
			 if ((srcFullRGB(g,h,0)!=srcToFillRGB(g,h,0))||
			     (srcFullRGB(g,h,1)!=srcToFillRGB(g,h,1))||
			     (srcFullRGB(g,h,2)!=srcToFillRGB(g,h,2)))
			      target_region(g,h)=1;
	       }
	       
               CP=tmp4;DP=tmp4;PP=tmp4;
               
               //Initialisation of source_region
	       tmp3=target_region;
               source_region=tmp3.dilate(size_dilate,size_dilate)-target_region;

               //Initialisation of fill_front
               tmp5=target_region;
	       fill_front=target_region-tmp5.erode(3,3);
 
	       
	       //my_display();
	       //ALGO
               run();
          }

          void run()
          {
               int IPS(0),JPS(0),IS(0),JS(0);
               CImg<float> SRC_prec(srcToFillCIE);
               SRC_prec.fill(0.0);
               calculate_Pp();
	       my_display();
               while(SRC_prec!=srcToFillCIE)
               {
                    get_maxPatch(&IPS,&JPS);
                    find_exemplar_Patch(IPS,JPS,&IS,&JS);
                    SRC_prec=srcToFillCIE;
                    fill_patch(IPS,JPS,IS,JS);//update srcToFillCIE
                    update_targetregion(&SRC_prec);
                    update_fillfront();
                    update_sourceregion();
                    calculate_Pp();
		    
		   // srcToFillRGB=srcToFillCIE.get_LabtoRGB();
		   // my_display();
                    
              }
              srcToFillRGB=srcToFillCIE.get_LabtoRGB();
	      my_display();
          }
          
          void calculate_Pp()
          {
               PP.fill(0.0);
               calculate_Cp();
               calculate_Dp();
               cimg_forXY(PP,x,y)
                    PP(x,y)=CP(x,y)*DP(x,y);
          }
          
          void calculate_Cp()
          {
               int i=floor(size_patch/2);
               CP.fill(0.0);
               cimg_forXY(fill_front,x,y)
                    if (fill_front(x,y)!=0)
                    { 
                         for(int z=-i;z<i+1;z++)
			 {
                              for(int t=-i;t<i+1;t++)
                                   if (target_region(x+z,y+t)==0)
                                        CP(x,y)++;
			 }
			 CP(x,y)/=size_patch*size_patch;
                    }
          }
	  
	  void binarization(CImg<bool> *in)
	  {
	       cimg_forXY(*in,x,y)
		    if (((*in)(x,y,0,0)!=0)||((*in)(x,y,0,1)!=0)||((*in)(x,y,0,2)!=0))
		    {
			 (*in)(x,y,0,0)=1;
			 (*in)(x,y,0,1)=1;
			 (*in)(x,y,0,2)=1;
		    }
	  }
                  
          void calculate_Dp() //TODO: toute la fonction!!
          {
                DP.fill(1.0);
          }
         
          void get_maxPatch(int *x,int *y)
          {
               float tmp(0.0);
               cimg_forXY(PP,a,b)
                    if (PP(a,b)>tmp)
                    {
                         tmp=PP(a,b);
                         *x=a;
                         *y=b;
                    }
          }
          
	  //update srcToFillRGB
          void fill_patch(const int & IPS,const int & JPS,const int & IS,const int & JS)
          {     
               //(IPS,JPS) : Coordonates of the higher priority front_fill pixel
               //(IS,JS)   : Coordonates of the center of the exemplar patch to find in source_region
               int f=floor(size_patch/2);
               for(int z=-f;z<f+1;z++)
                    for(int t=-f;t<f+1;t++)
                         if( (target_region(IPS+t,JPS+z)!=0) && (source_region(IS+t,JS+z)!=0) )//all pixels must be filled in the regions
			 {
			      srcToFillCIE(IPS+t,JPS+z,0)=srcToFillCIE(IS+t,JS+z,0);
			      srcToFillCIE(IPS+t,JPS+z,1)=srcToFillCIE(IS+t,JS+z,1);
			      srcToFillCIE(IPS+t,JPS+z,2)=srcToFillCIE(IS+t,JS+z,2);
			 }
          }
          
          void find_exemplar_Patch(int IPS,int JPS,int * IS, int *JS)
          {
               float TMP(0.0), SSD(1000000.0);
               int f=floor(size_patch/2);
	       bool ok=true;
               cimg_forXY(source_region,a,b)
               {
		    ok=true;
		    for(int z=-f;z<f+1;z++)
		    {
			 for(int t=-f;t<f+1;t++)
			      if(source_region(a+z,b+t) == 0)
				   ok=false;
		    }
		    if (ok)   
		    {
			 TMP=compare_patch(IPS,JPS, a, b);
			 if(TMP<SSD)
			 {
			      SSD=TMP;
			      *IS=a;
			      *JS=b;
			 }
                    }
               }
          }
          
           //(i,j) coordonates of the center of the patch to fill
          //(ii,jj) coordonates of the center of the patch to find in the source_region
          float compare_patch(const int i,const int j,const int ii,const int jj)
          {
               float res(0.0);
               int f=floor(size_patch/2);
               for(int z=-f;z<f+1;z++)
                    for(int t=-f;t<f+1;t++)
                         if ((target_region(i+t,j+z)!=0)&&(source_region(ii+t,jj+z)!=0))
			 {
			      res+=pow(srcToFillCIE(i+t,j+z,0)-srcToFillCIE(ii+t,jj+z,0),2)+
				   pow(srcToFillCIE(i+t,j+z,1)-srcToFillCIE(ii+t,jj+z,1),2)+
				   pow(srcToFillCIE(i+t,j+z,2)-srcToFillCIE(ii+t,jj+z,2),2);
			 }
               return res;
          }
	  
          void update_targetregion(CImg<float>  *Prec)
          {
	       CImg<bool> tmp(target_region);
	       tmp.fill(0);
	       cimg_forXY(srcToFillCIE,g,h)
			 if (((*Prec)(g,h,0)!=srcToFillCIE(g,h,0))||
			     ((*Prec)(g,h,1)!=srcToFillCIE(g,h,1))||
			     ((*Prec)(g,h,2)!=srcToFillCIE(g,h,2)))
			      tmp(g,h)=1;
               target_region-=tmp;
          }
	  
          void update_fillfront()
          {
               CImg<bool> tmp(target_region);
               fill_front=(target_region-tmp.erode(3,3));
          }
          
          void update_sourceregion()
          {
               CImg<bool> tmp(target_region);
               source_region=tmp.dilate(size_dilate,size_dilate)-target_region;
          }
          
          void my_display()
          {
               CImgDisplay 
	       //main_disp1_0(srcFullRGB , "Image full"),
               main_disp1_1(srcToFillRGB , "Image source to fill"),
               //main_disp1_3(target_region , "Target Region"),
               main_disp1_4(source_region , "Source Region");
               //main_disp1_5(fill_front    , "Fill Front"),
              // main_disp1_6(CP            , "CP");
               
               while (!main_disp1_1.button() && !main_disp1_1.is_closed()) {main_disp1_1.wait();} 
          }

};

#endif

