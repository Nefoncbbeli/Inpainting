#ifndef __CARIOU_PIMA_H__
#define __CARIOU_PIMA_H__

#include "tools.h"

class Pima
{
     protected:
	  CImg<unsigned char> srcFullRGB;
	  CImg<unsigned char> srcToFillRGB;
	  CImg<double> imageXYZ; 
	  CImg<double> imageLab;
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
	       CImg<bool> tmp3;
	       CImg<float> tmp4(tmp1.width(),tmp1.height(),1);
	       tmp4.fill(0);
	       
	       srcFullRGB=tmp1;
	       srcToFillRGB=tmp2;
	       size_dilate=_size_dilate;
	       size_patch=_size_patch;
	       alpha=_alpha;
	       
	       //Conversion RGB to CieLAB
	       imageXYZ=srcFullRGB.get_RGBtoXYZ(); 
	       imageLab=imageXYZ.get_XYZtoLab();

	       //Initialisation of target_region
	       target_region=(srcFullRGB-srcToFillRGB);
	       
	       CP=tmp4;
	       DP=tmp4;
	       PP=tmp4;
	       
	       //Initialisation of source_region
	       tmp3=target_region;
	       tmp3.dilate(size_dilate,size_dilate);
	       source_region=tmp3-target_region;
	       
	       //Initialisation of fill_front
	       fill_front=define_fillfront();
	       
	      
	       
	             
	       //ALGO
	       run();
	  }
	  
	  //update source_region & target_region
	  void update_region_to_fill()
	  {
	       
	  }
	  
	  
	  
	  void my_display()
	  {
	       CImgDisplay 
	       main_disp1_0(srcFullRGB    , "Image source full"),
	       main_disp1_1(srcToFillRGB  , "Image source to fill");
// 	       main_disp1_3(target_region , "Target Region"),
// 	       main_disp1_4(source_region , "Source Region"),
// 	       main_disp1_5(fill_front    , "Fill Front"),
// 	       main_disp1_6(CP            , "CP");
	       
	       while (!main_disp1_0.is_closed()) {main_disp1_0.wait();} 
	  }
	  
	  CImg<bool> define_fillfront()
	  {
	       CImg<bool> tmp(target_region);
	       CImg<bool> res(target_region-tmp.erode(3,3));
	       return res;	       
	  }
	  
	  void run()
	  {
	       int x(0),y(0);
	       //fill_front=define_fillfront();
	       //while fill_front!= 0;
		    calculate_Pp();
		    
		    
		    //Find Max_Patch
		    get_maxPatch(&x,&y);
		    //Find de examplar_patch
		    //Copy Patch
		    //update C(p)
	       //endwhile
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
	  
	  void get_maxPatch(int * x,int *y)
	  {
	       const unsigned char red[] ={255,0,0};
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
	       cout<<"to print ("<<*x<<","<<*y<<")"<<endl;
	       srcToFillRGB.draw_circle(*x,*y,20,red,3,5);
	  }
	  
	  
	  void calculate_Dp()
	  {
	        DP.fill(1.0);
	  }
	  
	  	  
};

#endif
