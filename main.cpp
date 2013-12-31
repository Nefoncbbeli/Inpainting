//Author : CARIOU ADRIEN
#include "pima.h"



int main(int argc, char** argv ) 
{
     /**
	*
	*Pima(image_source,image_retouchée,[taille_dilatation],[taille_patche],[reduction_influence],[seuil_rejet])
	*
	*argv[1] = image originale (non retouchée)
	*
	*argv[2] = image retouchée (zone à inpainter définie)
	*
	*taille_dilatation = taille de la  zone de recherche autour de la zone à inpainter (parametre optionnel, par défaut fixé à 25) 
	*
	*taille_patche = la taille des patches (parametre optionnel, par défaut fixé à 9)
	*
	*reduction_influence = double entre 0 et 1. 1=aucune réduction=>D(P) est prioritaire (parametre optionnel, par défaut fixé à 0.9)
	*
	*seuil_acceptation_d'erreur = double entre 0 et 100. 0=aucune erreur tolérée (ok pour images synthétiques) 
	*						(parametre optionnel, par défaut fixé à 2.0)
	*
	*
	*n.b : Pima(argv[1],argv[2])          équivalent à Pima(argv[1],argv[2],25,9,0.9,2.0)
	*      Pima(argv[1],argv[2],25)       équivalent à Pima(argv[1],argv[2],25,9,0.9,2.0)
	*      Pima(argv[1],argv[2],25,9)     équivalent à Pima(argv[1],argv[2],25,9,0.9,2.0)
	*      Pima(argv[1],argv[2],25,9,0.9) équivalent à Pima(argv[1],argv[2],25,9,0.9,2.0)
	*
	**/
	
     Pima(argv[1],argv[2],argv[3]);   //Parametres par défaut
	
	//copier ici les parametres recommandés pour les images désirées. Ils sont situés dans pima.sh 
	//Pima(argv[1],argv[2],80,9,0.98,2.0);
     
     return 0;
}
