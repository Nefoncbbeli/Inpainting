#! /bin/sh
make

#Chaque ligne contenant "./main "src1.ppm" "src2.ppm"
#peut être décommentée et tester directement.
#
#Les parametres proposés mis en commentaires, sont à copier 
#dans le fichier main.cpp pour de meilleurs résultats.
#In ne faut pas les décommenter.


#./main "lena1.ppm" "lena2.ppm"
##"Parametres par défaut : Pima(argv[1],argv[2]); 

#./main "ile1.ppm" "ile2.ppm"
##Pima(argv[1],argv[2],80,9,0.98,2.0);

./main "bw0.ppm" "bw1.ppm" 
#"Parametres par défaut : Pima(argv[1],argv[2]);

#./main "dam.ppm" "dam2.ppm" 
#Parametres idéaux : Pima(argv[1],argv[2],50,7,0.7,0.2);

#./main "damier.ppm" "damier4.ppm"
#"Parametres idéaux : Pima(argv[1],argv[2],30,5,0.7,0.2);

#./main "damier.ppm" "damier6.ppm"
#Parametres par défaut : Pima(argv[1],argv[2]);

#./main "kanizsa.ppm" "kanizsa2.ppm"
#parametres idéaux : Pima(argv[1],argv[2],30,9,0.9,0.1);

#./main "lac_full.ppm" "lac_full2.ppm"
#Parametres par défaut : Pima(argv[1],argv[2]);

#./main "japan.ppm" "japan2.ppm"
#Parametres par défaut : Pima(argv[1],argv[2]);

#./main "c.ppm" "d.ppm" #dilatation=700(toute l'image),patch_size=50  exemple qu'il retrouve bien le patch optimal SSD=0.0 partout!
#parametres idéaux : Pima(argv[1],argv[2],700,50);

#./main "poteau.ppm" "poteau2.ppm" # Prametre idéaux: dilatation=20 , taille patch=5 ,reduction_influence = 0.9, epsilon=2.0 
#parametres idéaux : Pima(argv[1],argv[2],20,5);

#./main "clip3.ppm" "clip4.ppm" 
#parametres idéaux : Pima(argv[1],argv[2],70,7);

#./main "clip_full.ppm" "clip_full2.ppm"  #Gros fichier!
#parametres idéaux : Pima(argv[1],argv[2],70,7);

#./main "jump1.ppm" "jump2.ppm"
#Parametres idéaux non trouvés... il faut surement revoir aussi "jump2.ppm"...

make mrproper