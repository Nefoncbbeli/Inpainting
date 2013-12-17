#include "pima.h"


Pima::Pima(const char * _srcFullRGB,const char * _srcToFillRGB, int _size_dilate,
	 double _size_patch,float _reduction_influence, double _epsilon) : 
srcFullRGB(), srcToFillRGB(), size_dilate(), size_patch(), reduction_influence(), epsilon() //constructeur
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
  nb_points=0;
  
  target_region=tmp3;
  source_region=tmp3;
  fill_front=tmp3;
  
  srcFullRGB=tmp1;
  srcToFillRGB=tmp2;
  size_dilate=_size_dilate;
  size_patch=_size_patch;
  
  srcToFillCIE=srcToFillRGB.get_RGBtoLab();
  
  reduction_influence=_reduction_influence;
  epsilon=_epsilon;
  
  //force size_patch to be odd
  if (((int)size_patch%2)==0) size_patch+=1.0;
  
  //Initialisation of target_region
  cimg_forXY(srcFullRGB,g,h){
    if ((srcFullRGB(g,h,0)!=srcToFillRGB(g,h,0))||
	   (srcFullRGB(g,h,1)!=srcToFillRGB(g,h,1))||
	   (srcFullRGB(g,h,2)!=srcToFillRGB(g,h,2)))
    {
	 target_region(g,h)=1;
	 nb_points++;
    }
  }
  nbtotaldepoint=nb_points;
  CP=tmp4;DP=tmp4;PP=tmp4;
  
  //Initialisation of source_region
  tmp3=target_region;
  source_region=tmp3.dilate(size_dilate,size_dilate)-target_region;
  
  //Initialisation of fill_front
  tmp5=target_region;
  fill_front=target_region-tmp5.erode(3,3);
  
  //ALGORITHME
  run();
}

void Pima::run()
{
  unsigned int IPS(0),JPS(0),IS(0),JS(0),cpt(0);
  double Cprec(0.0);
  float cpt2(0.9);
  CImg<unsigned char> SRC_prec(srcToFillRGB);
  SRC_prec.fill(0);
  initialize_Pp();
  my_display(cpt);
  while(nb_points!=0)//tant qu'il reste des pixels à remplir
  {	
    cpt++;
    Cprec=get_maxPatch(&IPS,&JPS);
    srcToFillCIE=srcToFillRGB.get_RGBtoLab();
    find_exemplar_Patch(IPS,JPS,&IS,&JS);
    SRC_prec=srcToFillRGB;
    fill_patch(IPS,JPS,IS,JS);
    update_targetregion(&SRC_prec,&cpt2);
    update_fillfront();
    update_sourceregion();
    update_Pp(IPS,JPS,Cprec);
    
    if (cpt%20==0)
    {
	 //my_display(cpt);   //Décommenter pour afficher le résultat toutes les 20 itérations!
    }
  }
  my_display(cpt);
  srcToFillRGB.save("Resultat.ppm",-1);
}
    
    
    
//Initialisation des priorités
void Pima::initialize_Pp()
{
  PP.fill(0.0);
  calculate_Dp();
  initialize_Cp();
  cimg_forXY(PP,x,y)
  {
    //PP(x,y)=DP(x,y); 		//Pour observer uniquement l'influence du terme d'attache aux données
    //PP(x,y)=CP(x,y);        //Pour observer uniquement l'influence du terme de confiance
    PP(x,y)=DP(x,y)*CP(x,y);
  }
}

//Mise à jour des priorités des pixels situés sur la frontière (fill_front)
void Pima::update_Pp(double _IPS,double _JPS,double _Cprec)
{
  PP.fill(0.0);
  update_Cp(_IPS,_JPS,_Cprec,reduction_influence);
  calculate_Dp();
  cimg_forXY(PP,x,y)
  {
    if((DP(x,y)<0)||(DP(x,y)>1))
	 cout<<"error normalisation DP ="<<DP(x,y)<<endl;
    if((CP(x,y)<0)||(CP(x,y)>1))
	 cout<<"error normalisation CP ="<<CP(x,y)<<endl;
    
    //PP(x,y)=DP(x,y);        //Pour observer uniquement l'influence du terme d'attache aux données
    //PP(x,y)=CP(x,y);        //Pour observer uniquement l'influence du terme de confiance
    PP(x,y)=DP(x,y)*CP(x,y);
  }
}

//Initialisation du terme de confiance C(p)
void Pima::initialize_Cp()
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
		cpt+=1.0;
    }
    CP(x,y)=cpt/((double)size_patch*(double)size_patch);
    cpt=0.0;
  }
}

//Mise à jour du terme de confiance C(p)
void Pima::update_Cp(const int & IPS,const int & JPS,const double & Cprec,const float reduction_influence)
{
  int i=floor(size_patch/2);
  for(int z=-i-1;z<i+2;z++)
  {
    for(int t=-i-1;t<i+2;t++)
    {
	 if ((CP(IPS+t,JPS+z)==0) && (fill_front(IPS+t,JPS+z)!=0))
	   CP(IPS+t,JPS+z)=Cprec*reduction_influence;
	 if ((CP(IPS+t,JPS+z)!=0) && (fill_front(IPS+t,JPS+z)==0))
	   CP(IPS+t,JPS+z)=0;
    }
  }
}

//Calcul du terme d'attache aux données D(p)
void Pima::calculate_Dp() 
{
  double gradXI[3]={0,0,0},gradYI[3]={0,0,0},npx(0),npy(0), mean[3]={0,0,0};
  double cpt(0);
  DP.fill(0.00);
  cimg_forXY(fill_front,x,y)
  {
    if(fill_front(x,y)!=0)
    {
	 if((x==1)||(y==1)||(x==W-1)||(y==H-1))
	   DP(x,y)=0.01;
	 else
	 {
	   
	   npx=target_region(x-1,y-1)+2*target_region(x-1,y)+target_region(x-1,y+1)-
	   target_region(x+1,y-1)-2*target_region(x+1,y)-target_region(x+1,y+1);
	   
	   npy=target_region(x-1,y-1)+2*target_region(x,y-1)+target_region(x+1,y-1)-
	   target_region(x-1,y+1)-2*target_region(x,y+1)-target_region(x+1,y+1);
	   
	   npx/=4.0;
	   npy/=4.0;
	   cpt=0.0;     
	   for(int z=-1;z<2;z++)
	   {
		for(int t=-1;t<2;t++)
		{
		  if (target_region(x+t,y+z)==0)
		  {
		    mean[0]+=srcToFillRGB(x+t,y+z,0);
		    mean[1]+=srcToFillRGB(x+t,y+z,1);
		    mean[2]+=srcToFillRGB(x+t,y+z,2);
		    cpt++;
		  }
		}
	   }
	   mean[0]/=cpt;mean[1]/=cpt;mean[2]/=cpt;
	   
	   //RED
	   gradYI[0] = ((target_region(x-1,y-1)==0) ? srcToFillRGB(x-1,y-1,0) : mean[0]) 
	   +2*((target_region(x-1,y)==0)   ? srcToFillRGB(x-1,y,0)   : mean[0])
	   + ((target_region(x-1,y+1)==0) ? srcToFillRGB(x-1,y+1,0) : mean[0])
	   - ((target_region(x+1,y-1)==0) ? srcToFillRGB(x+1,y-1,0) : mean[0])
	   -2*((target_region(x+1,y)==0)   ? srcToFillRGB(x+1,y,0)   : mean[0])     
	   - ((target_region(x+1,y+1)==0) ? srcToFillRGB(x+1,y+1,0) : mean[0]);
	   
	   gradXI[0] = ((target_region(x-1,y-1)==0) ? srcToFillRGB(x-1,y-1,0) : mean[0])
	   +2*((target_region(x,y-1)==0)   ? srcToFillRGB(x,y-1,0)   : mean[0])
	   + ((target_region(x+1,y-1)==0) ? srcToFillRGB(x+1,y-1,0) : mean[0]) 
	   - ((target_region(x-1,y+1)==0) ? srcToFillRGB(x-1,y+1,0) : mean[0])
	   -2*((target_region(x,y+1)==0)   ? srcToFillRGB(x,y+1,0)   : mean[0])
	   - ((target_region(x+1,y+1)==0) ? srcToFillRGB(x+1,y+1,0) : mean[0]);
	   
	   //GREEN   
	   gradYI[1] = ((target_region(x-1,y-1)==0) ? srcToFillRGB(x-1,y-1,1) : mean[1]) 
	   +2*((target_region(x-1,y)==0)   ? srcToFillRGB(x-1,y,1)   : mean[1])
	   + ((target_region(x-1,y+1)==0) ? srcToFillRGB(x-1,y+1,1) : mean[1])
	   - ((target_region(x+1,y-1)==0) ? srcToFillRGB(x+1,y-1,1) : mean[1])
	   -2*((target_region(x+1,y)==0)   ? srcToFillRGB(x+1,y,1)   : mean[1])     
	   - ((target_region(x+1,y+1)==0) ? srcToFillRGB(x+1,y+1,1) : mean[1]);
	   
	   gradXI[1] = ((target_region(x-1,y-1)==0) ? srcToFillRGB(x-1,y-1,1) : mean[1])
	   +2*((target_region(x,y-1)==0)   ? srcToFillRGB(x,y-1,1)   : mean[1])
	   +((target_region(x+1,y-1)==0) ? srcToFillRGB(x+1,y-1,1) : mean[1]) 
	   - ((target_region(x-1,y+1)==0) ? srcToFillRGB(x-1,y+1,1) : mean[1])
	   -2*((target_region(x,y+1)==0)   ? srcToFillRGB(x,y+1,1)   : mean[1])
	   - ((target_region(x+1,y+1)==0) ? srcToFillRGB(x+1,y+1,1) : mean[1]);
	   
	   //BLUE
	   gradYI[2] = ((target_region(x-1,y-1)==0) ? srcToFillRGB(x-1,y-1,2) : mean[2]) 
	   +2*((target_region(x-1,y)==0)   ? srcToFillRGB(x-1,y,2)   : mean[2])
	   + ((target_region(x-1,y+1)==0) ? srcToFillRGB(x-1,y+1,2) : mean[2])
	   - ((target_region(x+1,y-1)==0) ? srcToFillRGB(x+1,y-1,2) : mean[2])
	   -2*((target_region(x+1,y)==0)   ? srcToFillRGB(x+1,y,2)   : mean[2])     
	   - ((target_region(x+1,y+1)==0) ? srcToFillRGB(x+1,y+1,2) : mean[2]);
	   
	   gradXI[2] = ((target_region(x-1,y-1)==0) ? srcToFillRGB(x-1,y-1,2) : mean[2])
	   +2*((target_region(x,y-1)==0)   ? srcToFillRGB(x,y-1,2)   : mean[2])
	   + ((target_region(x+1,y-1)==0) ? srcToFillRGB(x+1,y-1,2) : mean[2]) 
	   - ((target_region(x-1,y+1)==0) ? srcToFillRGB(x-1,y+1,2) : mean[2])
	   -2*((target_region(x,y+1)==0)   ? srcToFillRGB(x,y+1,2)   : mean[2])
	   - ((target_region(x+1,y+1)==0) ? srcToFillRGB(x+1,y+1,2) : mean[2]);
	   
	   gradXI[0]/=-1020.0;gradXI[1]/=-1020.0;gradXI[2]/=-1020.0;  //Normalisation et rotation 
	   gradYI[0]/=1020.0;gradYI[1]/=1020.0;gradYI[2]/=1020.0;     //Normalisation 
	   
	   DP(x,y)=abs(gradXI[0]*npx+gradXI[1]*npx+gradXI[2]*npx
	   +gradYI[0]*npy+gradYI[1]*npy+gradYI[2]*npy)/3.0+0.01;
	 }
    }
  }
}

//Selectionne le pixel du contour ayant la priorité maximale
double Pima::get_maxPatch(unsigned int *x,unsigned int *y)
{
  double tmp(-1.0);
  cimg_forXY(PP,a,b)
  {
    if ( (fill_front(a,b) != 0) && (PP(a,b)>=tmp))
    {
	 tmp=PP(a,b);
	 *x=a;
	 *y=b;
    }
  }
  return CP(*x,*y);
}

//Selectionne le patch de la région de recherche ayant le moins de différences avec le patch du contour (SSD)
void Pima::find_exemplar_Patch(unsigned int IPS,unsigned int JPS,unsigned int * IS,unsigned int *JS)
{
  double TMP(0.0), SSD(1e10);
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
	 compare_patch(IPS,JPS, a, b,epsilon,&TMP);		 
	 if(TMP<=SSD)
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
//Calcul d'une SSD
inline void Pima::compare_patch(const int i,const int j,const int ii,const int jj,const double epsilon,double *TMP)
{
  
  double res(0.0),tmp(0.0),seuil(0.0);
  double mean0(0.0),mean1(0.0),mean2(0.0);
  double meanIN0(0.0),meanIN1(0.0),meanIN2(0.0);
  int f=floor(size_patch/2);
  unsigned int cpt(0);
  for(int z=-f;z<f+1;z++)
  {
    for(int t=-f;t<f+1;t++)
    {
	 mean0+=srcToFillCIE(ii+t,jj+z,0);
	 mean1+=srcToFillCIE(ii+t,jj+z,1);
	 mean2+=srcToFillCIE(ii+t,jj+z,2);	
	 
	 if (target_region(i+t,j+z)==0)
	 {
	   meanIN0+=srcToFillCIE(ii+t,jj+z,0);
	   meanIN1+=srcToFillCIE(ii+t,jj+z,1);
	   meanIN2+=srcToFillCIE(ii+t,jj+z,2);	
	   
	   cpt++;
	   tmp+=pow(srcToFillCIE(i+t,j+z,0)-srcToFillCIE(ii+t,jj+z,0),2)+
	   pow(srcToFillCIE(i+t,j+z,1)-srcToFillCIE(ii+t,jj+z,1),2)+
	   pow(srcToFillCIE(i+t,j+z,2)-srcToFillCIE(ii+t,jj+z,2),2);
	 }
    }
  }
  meanIN0/=(double)cpt;meanIN1/=(double)cpt;meanIN2/=(double)cpt;
  mean0/=(size_patch*size_patch);mean1/=(size_patch*size_patch);mean2/=(size_patch*size_patch);
  seuil=(abs(meanIN0-mean0)+abs(meanIN1-mean1)+abs(meanIN2-mean2));
  if(seuil<epsilon)//epsilon = seuil d'acceptation
    res=tmp;
  else res=tmp+1000;
  *TMP =res/(double)cpt;
  
}


//update srcToFillRGB
/*
*Copie les pixels du patch séléctionné dans le patch à inpainter(uniquement les pixels non déjà définis)
*/
void Pima::fill_patch(const int & IPS,const int & JPS,const int & IS,const int & JS)
{     
  //(IPS,JPS) : Coordonates of the higher priority front_fill pixel
  //(IS,JS)   : Coordonates of the center of the exemplar patch to find in source_region
  int f=floor(size_patch/2);
  for(int z=-f;z<f+1;z++)
    for(int t=-f;t<f+1;t++)
	 if( (target_region(IPS+t,JPS+z)!=0))
	 {
	   srcToFillRGB(IPS+t,JPS+z,0)=srcToFillRGB(IS+t,JS+z,0);
	   srcToFillRGB(IPS+t,JPS+z,1)=srcToFillRGB(IS+t,JS+z,1);
	   srcToFillRGB(IPS+t,JPS+z,2)=srcToFillRGB(IS+t,JS+z,2);
	 }
}

//mise à jour de la zone à inpainter
void Pima::update_targetregion(CImg<unsigned char>  *Prec,float *cpt2)
{
  double nbpourcent(nbtotaldepoint*(*cpt2));
  CImg<bool> tmp(target_region);
  tmp.fill(0);
  cimg_forXY(srcToFillRGB,g,h)
  {
    if ( ((*Prec)(g,h,0)!=srcToFillRGB(g,h,0))||
		((*Prec)(g,h,1)!=srcToFillRGB(g,h,1))||
		((*Prec)(g,h,2)!=srcToFillRGB(g,h,2))  )
    {
	 tmp(g,h)=1;
	 nb_points--;
	 
	 if (nb_points==(unsigned long int)nbpourcent)
	 {
	   cout<<"Etat de la progression : "<<(1-(*cpt2))*100<<"%"<<endl;
	   *cpt2-=0.1;
	   nbpourcent=nbtotaldepoint*(*cpt2);
	 }
    }
  }
  target_region-=tmp;
}

//mise à jour de la frontière
void Pima::update_fillfront()
{
  CImg<bool> tmp(target_region);
  fill_front=(target_region-tmp.erode(3,3));
}

//mise à jour de la région source
void Pima::update_sourceregion()
{
  CImg<bool> tmp(target_region);
  source_region=tmp.dilate(size_dilate,size_dilate)-target_region;
}

/*
*Fonction d'affichage
*
*Décommenter pour voir les différentes images générées
*
*n.b : srcToFillRGB doit toujours apparaitre,et attention aux virgules
*à la fin des "main_disp".Le dernier "main_disp" doit finir par un point-virgule 
*/
void Pima::my_display(unsigned int cpt)
{
  if(cpt==0)
    cout<<"Cliquez sur l'image pour lancer l'algorithme"<<endl;
  CImgDisplay 
  //main_disp1_0(srcFullRGB , "Image initiale"),
  main_disp1_1(srcToFillRGB , "__Cliquez sur l'image__");
  //main_disp1_3(target_region , "Target Region"),
  //main_disp1_4(source_region , "Source Region"),
  //main_disp1_5(fill_front    , "Fill Front"),
  //main_disp1_7(CP            , "CP"),
  //main_disp1_8(DP            , "DP"),
  //main_disp1_6(PP            , "PP");
  
  while (!main_disp1_1.button() && !main_disp1_1.is_closed()) {main_disp1_1.wait();} 
}