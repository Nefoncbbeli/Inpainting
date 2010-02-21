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
	  int size_dilate;
     
     
     public:

	  Pima() : srcFullRGB(),srcToFillRGB() {}
	  ~Pima() {}

	  Pima(const char * _srcFullRGB,const char * _srcToFillRGB, int _size_dilate=25) : srcFullRGB(), srcToFillRGB(), size_dilate()
	  {	
	       //initialisation 
	       CImg<unsigned char> tmp1(_srcFullRGB);
	       CImg<unsigned char> tmp2(_srcToFillRGB);
	       CImg<bool> tmp3;
	       
	       srcFullRGB=tmp1;
	       srcToFillRGB=tmp2;
	       size_dilate=_size_dilate;
	       
	       //Conversion RGB to CieLAB
	       imageXYZ=srcFullRGB.get_RGBtoXYZ(); 
	       imageLab=imageXYZ.get_XYZtoLab();

	       //Initialisation of target_region
	       target_region=(srcFullRGB-srcToFillRGB);
	       
	       //Initialisation of source_region
	       tmp3=target_region;
	       tmp3.dilate(size_dilate,size_dilate);
	       source_region=tmp3-target_region;
	       
	       //Initialisation of fill_front
	       fill_front=define_fillfront();
	       
	       
	       //Display
	       //my_display();
	       
	  }
	  
	  //update source_region & target_region
	  void update_region_to_fill()
	  {
	       
	  }
	  
	  
	  
	  void my_display()
	  {
	       CImgDisplay 
	       main_disp1_0(srcFullRGB    , "Image source full"),
	       main_disp1_1(srcToFillRGB  , "Image source to fill"),
	       main_disp1_3(target_region , "Target Region"),
	       main_disp1_4(source_region , "Source Region"),
	       main_disp1_5(fill_front    , "Fill Front");
	       
	       while (!main_disp1_0.is_closed()) {main_disp1_0.wait();} 
	  }
	  
	  CImg<bool> define_fillfront()
	  {
	       CImg<bool> tmp(target_region);
	       CImg<bool> res(target_region-tmp.erode(3,3));
	       return res;	       
	  }
	  
	  	  
};

#endif
