//Author : CARIOU ADRIEN
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
  
    
    unsigned long int nbtotaldepoint;
    unsigned long int nb_points;
    
    //parametres influents 
    int size_dilate;
	const char * outputName;
	double size_patch;
    float reduction_influence;
    double epsilon;  
    
  public:
    Pima() : srcFullRGB(), srcToFillRGB() {}
    ~Pima() {}
    
    Pima(
		const char * _srcFullRGB
	,	const char * _srcToFillRGB
	,	const char * _outputName="Result.ppm"
	,	int _size_dilate=25
	,	double _size_patch=9
	,	float _reduction_influence=0.9
	,	double _epsilon=2.0
	);

    //Coeur de l'algorithme
    void run(const char * _outputName);
    
    //Initialisation des priorites
    void initialize_Pp();

    //Mise à jour des priorités des pixels situés sur la frontière (fill_front)
    void update_Pp(double _IPS, double _JPS, double _Cprec);
    
    //Initialisation du terme de confiance C(p)
    void initialize_Cp();
    
    //Mise à jour du terme de confiance C(p)
    void update_Cp(const int & IPS,const int & JPS,const double & Cprec,const float reduction_influence);
    
    //Calcul du terme d'attache aux données D(p)
    void calculate_Dp();
    
    //Selectionne le pixel du contour ayant la priorité maximale
    double get_maxPatch(unsigned int *x,unsigned int *y);
    
    //Selectionne le patch de la région de recherche ayant le moins de différences avec le patch du contour (SSD)
    void find_exemplar_Patch(unsigned int IPS,unsigned int JPS,unsigned int * IS,unsigned int *JS);
    
    //(i,j) coordonates of the center of the patch to fill
    //(ii,jj) coordonates of the center of the patch to find in the source_region
    //Calcul d'une SSD
    inline void compare_patch(const int i,const int j,const int ii,const int jj,const double epsilon, double *TMP);    
    
    /*
    *Copie les pixels du patch séléctionné dans le patch à inpainter(uniquement les pixels non déjà définis)
    *Met a jour srcToFillRGB
    */
    void fill_patch(const int & IPS,const int & JPS,const int & IS,const int & JS);
    
    //mise à jour de la zone à inpainter
    void update_targetregion(CImg<unsigned char>  *Prec,float *cpt2);
    
    //mise à jour de la frontiere
    void update_fillfront();
    
    //mise à jour de la region source
    void update_sourceregion();
    
    //Fonction d'affichage
    void my_display(unsigned int cpt);
    
};

#endif

