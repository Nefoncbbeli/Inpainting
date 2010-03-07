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
          CImg<bool> target_region;
          CImg<bool> source_region;
          CImg<bool> fill_front;
          CImg<double> CP;//Confidence term
          CImg<double> DP;//Data term
          CImg<double> PP;//Priority values
          int size_dilate;
          double size_patch;
          double alpha;
	  
	 
     public:

          Pima() : srcFullRGB(),srcToFillRGB() {}
          ~Pima() {}

          Pima(const char * _srcFullRGB,const char * _srcToFillRGB, int _size_dilate=25, double _size_patch=9,int _alpha=255) : 
               srcFullRGB(), srcToFillRGB(), size_dilate(), size_patch(), alpha()
          {     
               //initialisation 
               
               CImg<unsigned char> tmp1(_srcFullRGB);
               CImg<unsigned char> tmp2(_srcToFillRGB);
	       W=tmp1.width(),H=tmp1.height();
               CImg<bool> tmp3(W,H,1);
               tmp3.fill(0);
               CImg<double> tmp4(W,H,1);
               tmp4.fill(0.0);
               CImg<bool> tmp5(W,H,1);
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
 
	       //ALGO
               run();
	       //float Sd = compare_patch(54,45,400,43);
	       //cout<<"SSD max = "<<Sd<<endl;
          }

          void run()
          {
               int IPS(0),JPS(0),IS(0),JS(0);
               CImg<unsigned char> SRC_prec(srcToFillRGB);
               SRC_prec.fill(0.0);
               initialize_Pp();
	       my_display();
	       double Cprec(0.0);
	       //unsigned char red[]={255,0,0};
	       //unsigned char blue[]={0,0,255};
               while(SRC_prec!=srcToFillRGB)
	       {	
                    Cprec=get_maxPatch(&IPS,&JPS);
		    srcToFillCIE=srcToFillRGB.get_RGBtoLab();
                    find_exemplar_Patch(IPS,JPS,&IS,&JS);
                    SRC_prec=srcToFillRGB;
                    fill_patch(IPS,JPS,IS,JS);//update srcToFillRGB
		    
                    update_targetregion(&SRC_prec);
                    update_fillfront();
                    update_sourceregion();
		    update_Pp(IPS,JPS,Cprec);
                    
		    //srcToFillRGB.draw_circle(IPS,JPS,10,blue,1,1);
		    //srcToFillRGB.draw_circle(IS,JS,10,red,1,1);
		    //my_display();
                    
              }
              //srcToFillRGB=srcToFillCIE.get_LabtoRGB();
	      my_display();
          }
          
          void update_Pp(double _IPS,double _JPS,double _Cprec)
          {
               PP.fill(0.0);
               update_Cp(_IPS,_JPS,_Cprec);
               calculate_Dp();
               cimg_forXY(PP,x,y)
	       {
                    //PP(x,y)=DP(x,y);
                    PP(x,y)=CP(x,y);
		    //PP(x,y)=DP(x,y)*CP(x,y)/pow(256,3);//TODO pb calcul DP & normalisation
		    
		    //if (PP(x,y)!=0)
			//cout<<"DP,CP "<<DP(x,y)<<" "<<CP(x,y)<<endl;	
	       }
          }
	  
	   void initialize_Pp()
          {
               PP.fill(0.0);
               calculate_Dp();
	       initialize_Cp();
               cimg_forXY(PP,x,y)
	       {
                    //PP(x,y)=DP(x,y);
                    PP(x,y)=CP(x,y);
		    //PP(x,y)=DP(x,y)*CP(x,y)/pow(256,3);//TODO pb calcul DP & normalisation
		  
	       }
          }
	  
	  void update_Cp(const int & IPS,const int & JPS,const double & Cprec)
	  {
	        int i=floor(size_patch/2);
	       for(int z=-i-1;z<i+2;z++)
	       {
		    for(int t=-i-1;t<i+2;t++)
		    {
			 if ((CP(IPS+t,JPS+z)==0) && (fill_front(IPS+t,JPS+z)!=0))
			      CP(IPS+t,JPS+z)=Cprec;
			 if ((CP(IPS+t,JPS+z)!=0) && (fill_front(IPS+t,JPS+z)==0))
			      CP(IPS+t,JPS+z)=0;
		    }
	       }
	  }
          
          void initialize_Cp()
          {
               int i=floor(size_patch/2);
               CP.fill(0.0);
	       double cpt(0.0);
               cimg_forXY(fill_front,x,y)
                    if (fill_front(x,y)!=0)
                    { 
                         for(int z=-i;z<i+1;z++)
			 {
                              for(int t=-i;t<i+1;t++)
                                   if (target_region(x+z,y+t)==0)
                                        cpt++;
			 }
			 CP(x,y)=cpt/pow(size_patch,2);//TODO pb normalisation
			 cpt=0.0;
			 //cout<<"CP(x,y) = "<<CP(x,y)<<endl;
                    }
          }
	 
                  
          void calculate_Dp() 
	  {
	      
	       double gradXI[3]={0,0,0},gradYI[3]={0,0,0},npx(0),npy(0), mean[3]={0,0,0};
	       double cpt(0);
	       DP.fill(0);
	       cimg_forXY(fill_front,x,y)
	       {
		    if(fill_front(x,y)!=0)
		    {
			 npx=fill_front(x-1,y-1)+2*fill_front(x-1,y)+fill_front(x-1,y+1)-
			     fill_front(x+1,y-1)-2*fill_front(x+1,y)-fill_front(x+1,y+1);
			     
			 npy=fill_front(x-1,y-1)+2*fill_front(x,y-1)+fill_front(x+1,y-1)-
			     fill_front(x-1,y+1)-2*fill_front(x,y+1)-fill_front(x+1,y+1);
			     
			 //npx/=4;
			 //npy/=4;
			      
			 for(int z=-1;z<2;z++)
			 {
                              for(int t=-1;t<2;t++)
			      {
				   if (source_region(x+t,y+z)!=0)
				   {
					mean[0]+=srcToFillRGB(x+t,y+z,0);
					mean[1]+=srcToFillRGB(x+t,y+z,1);
					mean[2]+=srcToFillRGB(x+t,y+z,2);
					cpt++;
				   }
			      }
			 }
			 //mean[0]/=(cpt*256);mean[1]/=(cpt*256);mean[2]/=(cpt*256);
			
			 gradYI[0] = ((source_region(x-1,y-1)!=0) ? srcToFillRGB(x-1,y-1,0) : mean[0]) 
			          +2*((source_region(x-1,y)!=0)   ? srcToFillRGB(x-1,y,0)   : mean[0])
			           + ((source_region(x-1,y+1)!=0) ? srcToFillRGB(x-1,y+1,0) : mean[0])
			           - ((source_region(x+1,y-1)!=0) ? srcToFillRGB(x+1,y-1,0) : mean[0])
			          -2*((source_region(x+1,y)!=0)   ? srcToFillRGB(x+1,y,0)   : mean[0])     
			           - ((source_region(x+1,y+1)!=0) ? srcToFillRGB(x+1,y+1,0) : mean[0]);
			     
			 gradXI[0]=  ((source_region(x-1,y-1)!=0) ? srcToFillRGB(x-1,y-1,0) : mean[0])
				  +2*((source_region(x,y-1)!=0)   ? srcToFillRGB(x,y-1,0)   : mean[0])
			           + ((source_region(x+1,y-1)!=0) ? srcToFillRGB(x+1,y-1,0) : mean[0]) 
			           - ((source_region(x-1,y+1)!=0) ? srcToFillRGB(x-1,y+1,0) : mean[0])
			          -2*((source_region(x,y+1)!=0)   ? srcToFillRGB(x,y+1,0)   : mean[0])
			           - ((source_region(x+1,y+1)!=0) ? srcToFillRGB(x+1,y+1,0) : mean[0]);
				   
				   
			 gradYI[1] = ((source_region(x-1,y-1)!=0) ? srcToFillRGB(x-1,y-1,1) : mean[1]) 
			          +2*((source_region(x-1,y)!=0)   ? srcToFillRGB(x-1,y,1)   : mean[1])
			           + ((source_region(x-1,y+1)!=0) ? srcToFillRGB(x-1,y+1,1) : mean[1])
			           - ((source_region(x+1,y-1)!=0) ? srcToFillRGB(x+1,y-1,1) : mean[1])
			          -2*((source_region(x+1,y)!=0)   ? srcToFillRGB(x+1,y,1)   : mean[1])     
			           - ((source_region(x+1,y+1)!=0) ? srcToFillRGB(x+1,y+1,1) : mean[1]);
			     
			 gradXI[1]=  ((source_region(x-1,y-1)!=0) ? srcToFillRGB(x-1,y-1,1) : mean[1])
				  +2*((source_region(x,y-1)!=0)   ? srcToFillRGB(x,y-1,1)   : mean[1])
			            +((source_region(x+1,y-1)!=0) ? srcToFillRGB(x+1,y-1,1) : mean[1]) 
			           - ((source_region(x-1,y+1)!=0) ? srcToFillRGB(x-1,y+1,1) : mean[1])
			          -2*((source_region(x,y+1)!=0)   ? srcToFillRGB(x,y+1,1)   : mean[1])
			           - ((source_region(x+1,y+1)!=0) ? srcToFillRGB(x+1,y+1,1) : mean[1]);
				   
				   
		         gradYI[2] = ((source_region(x-1,y-1)!=0) ? srcToFillRGB(x-1,y-1,2) : mean[2]) 
			          +2*((source_region(x-1,y)!=0)   ? srcToFillRGB(x-1,y,2)   : mean[2])
			           + ((source_region(x-1,y+1)!=0) ? srcToFillRGB(x-1,y+1,2) : mean[2])
			           - ((source_region(x+1,y-1)!=0) ? srcToFillRGB(x+1,y-1,2) : mean[2])
			          -2*((source_region(x+1,y)!=0)   ? srcToFillRGB(x+1,y,2)   : mean[2])     
			           - ((source_region(x+1,y+1)!=0) ? srcToFillRGB(x+1,y+1,2) : mean[2]);
			     
			 gradXI[2]=  ((source_region(x-1,y-1)!=0) ? srcToFillRGB(x-1,y-1,2) : mean[2])
				  +2*((source_region(x,y-1)!=0)   ? srcToFillRGB(x,y-1,2)   : mean[2])
			           + ((source_region(x+1,y-1)!=0) ? srcToFillRGB(x+1,y-1,2) : mean[2]) 
			           - ((source_region(x-1,y+1)!=0) ? srcToFillRGB(x-1,y+1,2) : mean[2])
			          -2*((source_region(x,y+1)!=0)   ? srcToFillRGB(x,y+1,2)   : mean[2])
			           - ((source_region(x+1,y+1)!=0) ? srcToFillRGB(x+1,y+1,2) : mean[2]);
				   

			 //gradXI[0]/=1024;gradXI[1]/=1024;gradXI[2]/=1024;//TODO pb normalisation
			 //gradYI[0]/=1024;gradYI[1]/=1024;gradYI[2]/=1024;
			 
			 gradXI[0]=-gradXI[0];gradXI[1]=-gradXI[1];gradXI[2]=-gradXI[2];
			      
			 DP(x,y)=abs(gradXI[0]*npx+gradXI[1]*npx+gradXI[2]*npx
				    +gradYI[0]*npy+gradYI[1]*npy+gradYI[2]*npy);
		    }
	       }
          }
          
          double get_maxPatch(int *x,int *y)
          {
               double tmp(-1.0);
               cimg_forXY(PP,a,b)
	       {
                    if ( (fill_front(a,b) != 0) && (PP(a,b)>tmp))
                    {
                         tmp=PP(a,b);
                         *x=a;
                         *y=b;
                    }
	       }
	       return CP(*x,*y);
          }

          
          void find_exemplar_Patch(int IPS,int JPS,int * IS, int *JS)
          {
               double TMP(0.0), SSD(1.0);
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
	       //cout<<"SSD = "<<SSD<<endl;
	       //cout<<"a-b = "<<IPS<<" "<<JPS<<endl;
	       //cout<<"a-b = "<<*IS<<" "<<*JS<<endl;
	       
          }
          
          //(i,j) coordonates of the center of the patch to fill
          //(ii,jj) coordonates of the center of the patch to find in the source_region
          float compare_patch(const int i,const int j,const int ii,const int jj)
          {
               double res(0.0);
               int f=floor(size_patch/2);
	       int cpt(0);
               for(int z=-f;z<f+1;z++)
	       {
                    for(int t=-f;t<f+1;t++)
                         if ((target_region(i+t,j+z)==0) && (source_region(ii+t,jj+z)!=0)) //inutile pour l'instant
			 {
			      
			      cpt++;
			      
			      res+=pow(srcToFillCIE(i+t,j+z,0)-srcToFillCIE(ii+t,jj+z,0),2)+
				   pow(srcToFillCIE(i+t,j+z,1)-srcToFillCIE(ii+t,jj+z,1),2)+
				   pow(srcToFillCIE(i+t,j+z,2)-srcToFillCIE(ii+t,jj+z,2),2);
				   /*
				   cout<<"srcToFillCIE(i+t,j+z,0) = "<<srcToFillCIE(i+t,j+z,0)<<endl;
				   cout<<"srcToFillCIE(ii+t,jj+z,0) = "<<srcToFillCIE(ii+t,jj+z,0)<<endl;
				   cout<<"srcToFillCIE(i+t,j+z,1) = "<<srcToFillCIE(i+t,j+z,1)<<endl;
				   cout<<"srcToFillCIE(ii+t,jj+z,1) = "<<srcToFillCIE(ii+t,jj+z,1)<<endl;
				   cout<<"srcToFillCIE(i+t,j+z,2) = "<<srcToFillCIE(i+t,j+z,2)<<endl;
				   cout<<"srcToFillCIE(ii+t,jj+z,2) = "<<srcToFillCIE(ii+t,jj+z,2)<<endl;
				   */
			      
			 }
	       }
	       return res;//(62800*cpt);//TODO: pb de normalisation
          }
	  
	            
	  //update srcToFillRGB
          void fill_patch(const int & IPS,const int & JPS,const int & IS,const int & JS)
          {     
               //(IPS,JPS) : Coordonates of the higher priority front_fill pixel
               //(IS,JS)   : Coordonates of the center of the exemplar patch to find in source_region
               int f=floor(size_patch/2);
               for(int z=-f;z<f+1;z++)
                    for(int t=-f;t<f+1;t++)
                         if( (target_region(IPS+t,JPS+z)!=0))// && (source_region(IS+t,JS+z)!=0) )//all pixels must be filled in the regions
			 {
			      srcToFillRGB(IPS+t,JPS+z,0)=srcToFillRGB(IS+t,JS+z,0);
			      srcToFillRGB(IPS+t,JPS+z,1)=srcToFillRGB(IS+t,JS+z,1);
			      srcToFillRGB(IPS+t,JPS+z,2)=srcToFillRGB(IS+t,JS+z,2);
			 }
          }
	  
          void update_targetregion(CImg<unsigned char>  *Prec)
          {
	       CImg<bool> tmp(target_region);
	       tmp.fill(0);
	       cimg_forXY(srcToFillRGB,g,h)
	       {
			 if ( ((*Prec)(g,h,0)!=srcToFillRGB(g,h,0))||
			      ((*Prec)(g,h,1)!=srcToFillRGB(g,h,1))||
			      ((*Prec)(g,h,2)!=srcToFillRGB(g,h,2))  )
			 {
			      tmp(g,h)=1;
			 }
	       }
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
               main_disp1_3(target_region , "Target Region"),
               main_disp1_4(source_region , "Source Region"),
               main_disp1_5(fill_front    , "Fill Front"),
               main_disp1_6(PP            , "PP");
               
               while (!main_disp1_1.button() && !main_disp1_1.is_closed()) {main_disp1_1.wait();} 
          }

};

#endif

