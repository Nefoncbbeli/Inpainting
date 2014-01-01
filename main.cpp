//Author : CARIOU ADRIEN
#include "pima.h"



int main(int argc, char** argv ) 
{
     /**
	*
	*Pima(
		input_image
	,	input_modified_image
	,	[output_path]
	,	[dilatation_size]
	,	[patch_size]
	,	[influence_reduction]
	,	[reject_threshold])
	*
	*argv[1] = input_image (path of the original picture ; mandatory)
	*
	*argv[2] = input_modified_image (path of the image with a modify area wich is the one to "inpaint" ; mandatory)
	*
	*argv[3] = output_path (output path ; optional, default = "./result.ppm" )
	*
	*dilatation_size = size in pixel of the research area around the inpaint zone (optional, default = 25) 
	*
	*patch_size = zise of the patch (optional, default = 9)
	*
	*influence_reduction = double between 0 and 1 : 1 = no reduction =>priority for D(p) (optional, default = 0.9)
	*
	*reject_threshold = double between 0 and 100. 0 = no error (only good for non real images ; optional, default = 2.0)
	*
	*
	*n.b : Pima(argv[1],argv[2])          équivalent à Pima(argv[1],argv[2],"./result.ppm",25,9,0.9,2.0)
	*      Pima(argv[1],argv[2],25)       équivalent à Pima(argv[1],argv[2],"./result.ppm",25,9,0.9,2.0)
	*      Pima(argv[1],argv[2],25,9)     équivalent à Pima(argv[1],argv[2],"./result.ppm",25,9,0.9,2.0)
	*      Pima(argv[1],argv[2],25,9,0.9) équivalent à Pima(argv[1],argv[2],"./result.ppm",25,9,0.9,2.0)
	*
	**/
	
     Pima(argv[1], argv[2], argv[3]);

	//copy here recommanded params for wanted images. Some examples are in pima.sh 
	//Pima(argv[1],argv[2],80,9,0.98,2.0);
     
     return 0;
}
