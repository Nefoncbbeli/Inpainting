#ifndef __CARIOU_PIMA_H__
#define __CARIOU_PIMA_H__

#include "tools.h"

class Pima
{
     protected:
	  CImg<unsigned char> srcFullRGB;
	  CImg<unsigned char> srcFullCIE;
	  CImg<unsigned char> srcToFillCIE;  
	  CImg<unsigned char> srcToFillRGB;	       
	 
	  CImg<double> imageXYZ; 
	  CImg<bool> target_region;
	  CImg<bool> source_region;
	  CImg<bool> fill_front;
	  CImg<double> CP;//Confidence term
	  CImg<double> DP;//Data term
	  CImg<double> PP;//Priority values
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
	       double W=tmp1.width(),H=tmp1.height();
	       CImg<bool> tmp3(W,H,1);
	       tmp3.fill(0);
	       CImg<float> tmp4(W,H,1);
	       tmp4.fill(0.0);
	       CImg<bool> tmp5(W,H,1);
	       tmp5.fill(0);
	       
	       
	       srcFullRGB=tmp1;
	       srcToFillRGB=tmp2;
	       size_dilate=_size_dilate;
	       size_patch=_size_patch;
	       alpha=_alpha;
	       
	       //Conversion RGB to CieLAB
// 	       imageXYZ=srcFullRGB.get_RGBtoXYZ(); 
// 	       srcFullCIE =imageXYZ.get_XYZtoLab();
	       imageXYZ=srcToFillRGB.get_RGBtoXYZ();
	       srcToFillCIE=imageXYZ.get_XYZtoLab();

	       //Initialisation of target_region
	       target_region=(srcFullRGB-srcToFillRGB);
	       
	       CP=tmp4;
	       DP=tmp4;
	       PP=tmp4;
	       
	       //Initialisation of source_region
	       tmp3=target_region;
	       source_region=tmp3.dilate(size_dilate,size_dilate)-target_region;
	       
	       //Initialisation of fill_front
	       tmp5=target_region;
	       fill_front=target_region-tmp5.erode(3,3);
	       
	       //ALGO
	       run();
	  }

	  void run()
	  {
	       int IPS(0),JPS(0),IS(0),JS(0);
	       CImg<unsigned char> SRC_prec(srcToFillRGB);
	       SRC_prec.fill(0);
	       calculate_Pp();//OK
	      
	      while(SRC_prec!=srcToFillRGB)
	      {
	     	   
		    get_maxPatch(&IPS,&JPS);//ok
		    find_exemplar_Patch(IPS,JPS,&IS,&JS);//ok
		    //cout<<"to print ("<<IPS<<","<<JPS<<")"<<endl;
		    //cout<<"to choose ("<<IS<<","<<JS<<")"<<endl;cout<<""<<endl;
		    SRC_prec=srcToFillRGB;
		    fill_patch(IPS,JPS,IS,JS);
		    
		    update_targetregion(&SRC_prec);
		    update_fillfront();
		    update_sourceregion();
		    calculate_Pp();//OK
		    imageXYZ=srcToFillRGB.get_RGBtoXYZ();
		    srcToFillCIE=imageXYZ.get_XYZtoLab();
		    
	      }
	      my_display();
	  }
	  
	  void calculate_Pp()
	  {
	       PP.fill(0.0);
	       calculate_Cp();
	       calculate_Dp();
	       cimg_forXY(PP,x,y){
		    PP(x,y)=CP(x,y)*DP(x,y);
	       }
	  }
	  
	  void calculate_Cp()
	  {
	       int i=floor(size_patch/2);
	       CP.fill(0.0);
	       cimg_forXY(fill_front,x,y)
	       {
		    if (fill_front(x,y)==1)
		    { 
			 for(int z=-i;z<i+1;z++)
			      for(int t=-i;t<i+1;t++)
				   if (target_region(x+z,y+t)==0)
					CP(x,y)++;
			 
		    CP(x,y)/=size_patch*size_patch;
		    }
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
	       {
		    if (PP(a,b)>tmp)
		    {
			 tmp=PP(a,b);
			 *x=a;
			 *y=b;
		    }
	       }
	  }
	  
	  void fill_patch(const int & IPS,const int & JPS,const int & IS,const int & JS)
	  {	
	       //(IPS,JPS) : Coordonates of the higher priority front_fill pixel
	       //(IS,JS)   : Coordonates of the center of the exemplar patch to find in source_region
	       int f=floor(size_patch/2);
	       for(int z=-f;z<f+1;z++)
		    for(int t=-f;t<f+1;t++)
			 if((source_region(IS+t,JS+z) !=0)&&(target_region(IPS+t,JPS+z)!=0))//all pixels must be filled in the regions
			 {     //srcToFillRGB(IPS+t,JPS+z)=srcToFillRGB(IS+t,JS+z);
			     
			      srcToFillRGB(IPS+t,JPS+z,0,0)=srcToFillRGB(IS+t,JS+z,0,0);
			      srcToFillRGB(IPS+t,JPS+z,0,1)=srcToFillRGB(IS+t,JS+z,0,1);
			      srcToFillRGB(IPS+t,JPS+z,0,2)=srcToFillRGB(IS+t,JS+z,0,2);
			 }    
	  }
	  
	  void find_exemplar_Patch(int IPS,int JPS,int * IS, int *JS)
	  {
	       double TMP(0.0), SSD(10000.0);
	       int f=floor(size_patch/2);
	       cimg_forXY(source_region,a,b)
	       {
		    if(source_region(a,b) != 0)
		    {			 
			 if ((a>f-1) && (a<source_region.width()-f)
			     && (b>f-1) && (b<source_region.height()-f))
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
	  }
	  
	   //(i,j) coordonates of the center of the patch to fill
	  //(ii,jj) coordonates of the center of the patch to find in the source_region
	  float compare_patch(const int i,const int j,const int ii,const int jj)
	  {
	       float res(0.0);
	       int f=floor(size_patch/2);
	       for(int z=-f;z<f+1;z++)
		    for(int t=-f;t<f+1;t++)
			 if ((source_region(ii+t,jj+z)!=0)&&(target_region(i+t,j+z)!=0))
			 {
			      //res+=((srcToFillCIE(i+t,j+z)-srcToFillCIE(ii+t,jj+z))^2);
			 
			      res+=(((srcToFillCIE(i+t,j+z,0,0)-srcToFillCIE(ii+t,jj+z,0,0))^2)+
				    ((srcToFillCIE(i+t,j+z,0,1)-srcToFillCIE(ii+t,jj+z,0,1))^2)+
				    ((srcToFillCIE(i+t,j+z,0,2)-srcToFillCIE(ii+t,jj+z,0,2))^2));//SSD
			 
			 /*
			      res+=(((srcToFillRGB(i+t,j+z,0,0)-srcToFillRGB(ii+t,jj+z,0,0))^2)+
				    ((srcToFillRGB(i+t,j+z,0,1)-srcToFillRGB(ii+t,jj+z,0,1))^2)+
				    ((srcToFillRGB(i+t,j+z,0,2)-srcToFillRGB(ii+t,jj+z,0,2))^2));
			 */
			 }
	       return res;
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
	  
	  void update_targetregion(CImg<unsigned char>  *Prec)
	  {
	       CImg<bool> tmp(target_region);
	       tmp.fill(0);
	       tmp=srcToFillRGB-*Prec;
	       target_region-=tmp;
	       
	       //target_region-=(srcToFillRGB-*Prec);//TODO:pourquoi marche pas??
	  }
	  
	  void my_display()
	  {
	       CImgDisplay 
	      // main_disp1_0(srcFullRGB    , "Image source full"),
	       main_disp1_1(srcToFillRGB , "Image source to fill"),
	       main_disp1_3(target_region , "Target Region");
	       /*
	       main_disp1_4(source_region , "Source Region"),
	       main_disp1_5(fill_front    , "Fill Front"),
	       main_disp1_6(CP            , "CP");
	       */
	       
	       while (!main_disp1_1.is_closed()) {main_disp1_1.wait();} 
	  }

};

#endif
