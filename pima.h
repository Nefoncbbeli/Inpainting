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
	  CImg<bool> imagehole;
     
     
     public:

	  Pima() : srcFullRGB(),srcToFillRGB() {}
	  ~Pima() {}

	       Pima(const char * fileIN1,const char * fileIN2) : srcFullRGB(), srcToFillRGB()
	  {	
	       //initialisation des parametres
	       CImg<unsigned char> tmp1(fileIN1);
	       CImg<unsigned char> tmp2(fileIN2);
	       srcFullRGB=tmp1;
	       srcToFillRGB=tmp2;
	       imageXYZ=srcFullRGB.get_RGBtoXYZ(); 
	       imageLab=imageXYZ.get_XYZtoLab();
	       
	       //Remplissage de imagehole
	       imagehole=(srcFullRGB-srcToFillRGB);
	       
	       //Display
	       CImgDisplay main_disp1_0(srcFullRGB,"Image RGB"),
	       main_disp1_1(srcToFillRGB,"Image RGB"),
	       main_disp1_3(imagehole,"Image Hole");
	       //main_disp2(imageXYZ,"Image XYZ"),
	       //main_disp3(imageLab,"Image Lab");
	       while (!main_disp1_0.is_closed()) {main_disp1_0.wait();}
	       
	  }
};

#endif
