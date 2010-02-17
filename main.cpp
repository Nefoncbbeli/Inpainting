 #include <stdio.h>
 #include <stdlib.h>
 #include <iostream>
 #include "CImg.h"
 
  using namespace cimg_library;
  using namespace std;

int main() {
  CImg<unsigned char> image("lena.jpg"), visu(512,512,1,3,0);
  const unsigned char red[] = { 255,0,0 }, green[] = { 0,255,0 }, blue[] = { 0,0,255 };
  image.blur(2.5);
  CImgDisplay main_disp(image,"Click a point"), draw_disp(visu,"Intensity profile");
  while (!main_disp.is_closed() && !draw_disp.is_closed()) 
  {
    main_disp.wait();
    if (main_disp.button() && main_disp.mouse_y()>=0) 
    {
      const int y = main_disp.mouse_y();
      visu.fill(0).draw_graph(image.get_crop(0,y,0,0,image.width()-1,y,0,0),red,1,1,0,255,0);
      visu.draw_graph(image.get_crop(0,y,0,1,image.width()-1,y,0,1),green,1,1,0,255,0);
      visu.draw_graph(image.get_crop(0,y,0,2,image.width()-1,y,0,2),blue,1,1,0,255,0).display(draw_disp);
    }
  }
  cout<<"Hello World!"<<endl;
  cout<<"Hello World! AGAIN!!"<<endl;
  return 0;
}