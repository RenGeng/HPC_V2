#include <stdio.h>
#include <math.h>
#include <shalw.h>
#include <export.h>
#include <mpi.h>
#include <stdlib.h>


#define TAG_HPHY_LIGNE 1
#define TAG_UPHY_LIGNE 2
#define TAG_VPHY_LIGNE 3
#define TAG_HPHY_COLONNE 4
#define TAG_UPHY_COLONNE 5
#define TAG_VPHY_COLONNE 6


double hFil_forward(int t, int i, int j) {
  //Phase d'initialisation du filtre
  //HPHY(t - 1, i, j) est encore nul
  if (t <= 2)
    return HPHY(t, i, j);
  return HPHY(t - 1, i, j) +
    alpha * (HFIL(t - 1, i-1*(my_rank>=sqrt(np)), j-1*(my_rank%(int)sqrt(np)!=0)) - 2 * HPHY(t - 1, i, j) + HPHY(t, i, j));
}

double uFil_forward(int t, int i, int j) {
  //Phase d'initialisation du filtre
  //UPHY(t - 1, i, j) est encore nul
  if (t <= 2)
    return UPHY(t, i, j);
  return UPHY(t - 1, i, j) +
    alpha * (UFIL(t - 1, i, j) - 2 * UPHY(t - 1, i, j) + UPHY(t, i, j));
}

double vFil_forward(int t, int i, int j) {
  //Phase d'initialisation du filtre
  //VPHY(t - 1, i, j) est encore nul
  if (t <= 2)
    return VPHY(t, i, j);
  return VPHY(t - 1, i, j) +
    alpha * (VFIL(t - 1, i, j) - 2 * VPHY(t - 1, i, j) + VPHY(t, i, j));
}

double hPhy_forward(int t, int i, int j) {
  double c, d;
  
  c = 0.;
  if (i > 0)
    c = UPHY(t - 1, i - 1, j);

  d = 0.;
  if (j < size_y - 1)
    d = VPHY(t - 1, i, j + 1);

  return HFIL(t - 1, i-1*(my_rank>=sqrt(np)), j-1*(my_rank%(int)sqrt(np)!=0)) -
    dt * hmoy * ((UPHY(t - 1, i, j) - c) / dx +
		 (d - VPHY(t - 1, i, j)) / dy);
}

double uPhy_forward(int t, int i, int j) {
  double b, e, f, g;
  
  if (i == size_x - 1)
    return 0.;

  b = 0.;
  if (i < size_x - 1)
    b = HPHY(t - 1, i + 1, j);

  e = 0.;
  if (j < size_y - 1)
    e = VPHY(t - 1, i, j + 1);

  f = 0.;
  if (i < size_x - 1)
    f = VPHY(t - 1, i + 1, j);

  g = 0.;
  if (i < size_x - 1 && j < size_y - 1)
    g = VPHY(t - 1, i + 1, j + 1);

  return UFIL(t - 1, i, j) +
    dt * ((-grav / dx) * (b - HPHY(t - 1, i, j)) +
	  (pcor / 4.) * (VPHY(t - 1, i, j) + e + f + g) -
	  (dissip * UFIL(t - 1, i, j)));
}

double vPhy_forward(int t, int i, int j) {
  double c, d, e, f;

  if (j == 0)
    return 0.;

  c = 0.;
  if (j > 0)
    c = HPHY(t - 1, i, j - 1);

  d = 0.;
  if (i > 0 && j > 0)
    d = UPHY(t - 1, i -1, j -1);

  e = 0.;
  if (i > 0)
    e = UPHY(t - 1, i - 1, j);

  f = 0.;
  if (j > 0)
    f = UPHY(t - 1, i, j - 1);

  return VFIL(t - 1, i, j) +
    dt * ((-grav / dy) * (HPHY(t - 1, i, j) - c) -
	  (pcor / 4.) * (d + e + f + UPHY(t - 1, i, j)) -
	  (dissip * VFIL(t - 1, i, j)));
}

void forward(void) {
  FILE *file = NULL;
  double svdt = 0.;
  int t = 0;
  MPI_Status status;

  MPI_Datatype colonne,bloc_envoie;
  MPI_Type_vector(height_bloc,1,size_y,MPI_DOUBLE,&colonne);
  MPI_Type_commit(&colonne);


  	MPI_Datatype bloc_gather,bloc_test;
  		// int size[1] = {global_size_x*global_size_y};
  		// int subsize[1] = {height_bloc*width_bloc};
  		// int start[1] = {0};
  	if(my_rank==0)
  	{
  		MPI_Type_vector(height_bloc,width_bloc,global_size_y,MPI_DOUBLE,&bloc_test);
  		// MPI_Type_vector(size_x,size_y,global_size_y,MPI_DOUBLE,&bloc_test);
	    MPI_Type_create_resized(bloc_test, 0, sizeof(double), &bloc_gather);
  		MPI_Type_commit(&bloc_gather);
  	}





  	int disp[np],count[np];
  	if(my_rank==0)
  	{
	  	disp[0] = 0;
	  	// disp[1] = 128;
	  	// disp[2] = 32768;
	  	// disp[3] = 32896;
	  	count[0] = 1;

	  	for(int i = 1;i<np;i++)
	  	{
	  		count[i] = 1;



	  		// disp[i] = i*128;
	  		if(i%(int)sqrt(np)==0) disp[i] = i/(int)sqrt(np) * global_size_y * height_bloc;
	  		else disp[i] = disp[i-1] + width_bloc;

	  		// if(i%(int)sqrt(np)==0) disp[i] =disp[i-(int)sqrt(np)]+height_bloc*sqrt(np);
	  		// else disp[i] = disp[i-1]+1;


	  		// printf("disp[%d] = %d\n",i,disp[i]);
	  		// printf("count[%d] = %d\n",i,count[i]);
	  	}

  	}

  	
  if(my_rank%(int)sqrt(np)==0 || my_rank%(int)sqrt(np)==sqrt(np)-1)
  {
  	MPI_Type_vector(height_bloc,width_bloc,1,MPI_DOUBLE,&bloc_test);
  	MPI_Type_create_resized(bloc_test, 0,  sizeof(double), &bloc_envoie);
  	MPI_Type_commit(&bloc_envoie);
  }
  else
  {
  	MPI_Type_vector(height_bloc,width_bloc,2,MPI_DOUBLE,&bloc_test);
  	MPI_Type_create_resized(bloc_test, 0, sizeof(double), &bloc_envoie);
  	MPI_Type_commit(&bloc_envoie);
  }
 // MPI_Gatherv(&HFIL(t,0,0),height_bloc*width_bloc,MPI_DOUBLE,&HFIL_global(t,0,0),count,disp,bloc_gather,0,MPI_COMM_WORLD);

  	 MPI_Gatherv(&HFIL(t,0,0),height_bloc*width_bloc,MPI_DOUBLE,&HFIL_global(t,0,0),count,disp,bloc_gather,0,MPI_COMM_WORLD);
  	 // MPI_Gatherv(&HFIL(t,(my_rank<sqrt(np) ? 0:1),(my_rank%(int)sqrt(np)==0) ? 0:1),height_bloc*width_bloc,MPI_DOUBLE,&HFIL_global(t,0,0),count,disp,bloc_gather,0,MPI_COMM_WORLD);

  if (file_export && my_rank == 0) 
    {
      file = create_file();
      export_step(file, t);
    }
   

  
  for (t = 1; t < nb_steps; t++) 
  {  
      
        if (t == 1) 
  	{
  	  svdt = dt;
  	  dt = 0;
  	}
        if (t == 2)
  	{
  	  dt = svdt / 2.;
  	}
    
       for (int j = (my_rank%(int)sqrt(np)==0) ? 0:1; j < (my_rank%(int)sqrt(np)==0)*width_bloc + (my_rank%(int)sqrt(np)!=0)*(width_bloc+1); j++)
  	{

  	  for (int i = (my_rank<sqrt(np)) ? 0:1; i <(my_rank<sqrt(np))*height_bloc + (my_rank>=sqrt(np))*(height_bloc+1);  i++)
  	    {   
  	      HPHY(t, i, j) = hPhy_forward(t, i, j);
  	      UPHY(t, i, j) = uPhy_forward(t, i, j);
  	      VPHY(t, i, j) = vPhy_forward(t, i, j);
  	      // HFIL(t, i, j) = hFil_forward(t, i, j);
  	      HFIL(t, i-1*(my_rank>=sqrt(np)), j-1*(my_rank%(int)sqrt(np)!=0))= hFil_forward(t, i, j);

  	      UFIL(t, i, j) = uFil_forward(t, i, j);
  	      VFIL(t, i, j) = vFil_forward(t, i, j);
  	    }
  	}

  	if(my_rank>=sqrt(np))
  	{
  		// MPI_Send(&HPHY(t,1,(my_rank%(int)sqrt(np)==0) ? 0:1),(my_rank%(int)sqrt(np)==0)*width_bloc + (my_rank%(int)sqrt(np)!=0)*(width_bloc+1),MPI_DOUBLE,my_rank-sqrt(np),TAG_HPHY_LIGNE,MPI_COMM_WORLD);
  		// MPI_Send(&VPHY(t,1,(my_rank%(int)sqrt(np)==0) ? 0:1),(my_rank%(int)sqrt(np)==0)*width_bloc + (my_rank%(int)sqrt(np)!=0)*(width_bloc+1),MPI_DOUBLE,my_rank-sqrt(np),TAG_VPHY_LIGNE,MPI_COMM_WORLD);
  		// MPI_Recv(&UPHY(t,0,(my_rank%(int)sqrt(np)==0) ? 0:1),(my_rank%(int)sqrt(np)==0)*width_bloc + (my_rank%(int)sqrt(np)!=0)*(width_bloc+1),MPI_DOUBLE,my_rank-sqrt(np),TAG_UPHY_LIGNE,MPI_COMM_WORLD,&status);

  		MPI_Send(&HPHY(t,1,(my_rank%(int)sqrt(np)==0) ? 0:1),width_bloc,MPI_DOUBLE,my_rank-(int)sqrt(np),TAG_HPHY_LIGNE,MPI_COMM_WORLD);
  		MPI_Send(&VPHY(t,1,(my_rank%(int)sqrt(np)==0) ? 0:1),width_bloc,MPI_DOUBLE,my_rank-(int)sqrt(np),TAG_VPHY_LIGNE,MPI_COMM_WORLD);
  		MPI_Recv(&UPHY(t,0,(my_rank%(int)sqrt(np)==0) ? 0:1),width_bloc,MPI_DOUBLE,my_rank-(int)sqrt(np),TAG_UPHY_LIGNE,MPI_COMM_WORLD,&status);

  		// printf("Process %d a envoyé à %d HPHY:%lf\n",my_rank,my_rank-(int)sqrt(np),HPHY(t,1,(my_rank%(int)sqrt(np)==0) ? 0:1));
  	}

  	if(my_rank<np-sqrt(np))
  	{
  		// MPI_Send(&UPHY(t,(my_rank<sqrt(np))*(height_bloc-1) + (my_rank>=sqrt(np))*height_bloc,(my_rank%(int)sqrt(np)==0) ? 0:1),(my_rank%(int)sqrt(np)==0)*width_bloc + (my_rank%(int)sqrt(np)!=0)*(width_bloc+1),MPI_DOUBLE,my_rank+sqrt(np),TAG_UPHY_LIGNE,MPI_COMM_WORLD);
  		// MPI_Recv(&HPHY(t,(my_rank<sqrt(np))*height_bloc + (my_rank>=sqrt(np))*(height_bloc+1),(my_rank%(int)sqrt(np)==0) ? 0:1),(my_rank%(int)sqrt(np)==0)*width_bloc + (my_rank%(int)sqrt(np)!=0)*(width_bloc+1),MPI_DOUBLE,my_rank+sqrt(np),TAG_HPHY_LIGNE,MPI_COMM_WORLD,&status);
  		// MPI_Recv(&VPHY(t,(my_rank<sqrt(np))*height_bloc + (my_rank>=sqrt(np))*(height_bloc+1),(my_rank%(int)sqrt(np)==0) ? 0:1),(my_rank%(int)sqrt(np)==0)*width_bloc + (my_rank%(int)sqrt(np)!=0)*(width_bloc+1),MPI_DOUBLE,my_rank+sqrt(np),TAG_VPHY_LIGNE,MPI_COMM_WORLD,&status);
  		
  		MPI_Send(&UPHY(t,size_x-2,(my_rank%(int)sqrt(np)==0) ? 0:1),width_bloc,MPI_DOUBLE,my_rank+(int)sqrt(np),TAG_UPHY_LIGNE,MPI_COMM_WORLD);
  		MPI_Recv(&HPHY(t,size_x-1,(my_rank%(int)sqrt(np)==0) ? 0:1),width_bloc,MPI_DOUBLE,my_rank+(int)sqrt(np),TAG_HPHY_LIGNE,MPI_COMM_WORLD,&status);
  		MPI_Recv(&VPHY(t,size_x-1,(my_rank%(int)sqrt(np)==0) ? 0:1),width_bloc,MPI_DOUBLE,my_rank+(int)sqrt(np),TAG_VPHY_LIGNE,MPI_COMM_WORLD,&status);

  		// printf("Process %d a reçu HPHY:%lf\n",my_rank,HPHY(t,size_x-1,(my_rank%(int)sqrt(np)==0) ? 0:1));
  	}

  	if(my_rank%(int)sqrt(np)!=sqrt(np)-1)
  	{
  		MPI_Send(&HPHY(t,(my_rank<sqrt(np) ? 0:1),size_y-2),1,colonne,my_rank+1,TAG_HPHY_COLONNE,MPI_COMM_WORLD);
  		MPI_Send(&UPHY(t,(my_rank<sqrt(np) ? 0:1),size_y-2),1,colonne,my_rank+1,TAG_UPHY_COLONNE,MPI_COMM_WORLD);
  		MPI_Recv(&VPHY(t,(my_rank<sqrt(np) ? 0:1),size_y-1),1,colonne,my_rank+1,TAG_VPHY_COLONNE,MPI_COMM_WORLD,&status);

  		// printf("Process %d a envoyé à process %d HPHY:%lf\n",my_rank,my_rank+1,HPHY(t,30,size_y-2));
  	}

  	if(my_rank%(int)sqrt(np)!=0)
  	{
  		MPI_Send(&VPHY(t,(my_rank<sqrt(np) ? 0:1),1),1,colonne,my_rank-1,TAG_VPHY_COLONNE,MPI_COMM_WORLD);
  		MPI_Recv(&HPHY(t,(my_rank<sqrt(np) ? 0:1),0),1,colonne,my_rank-1,TAG_HPHY_COLONNE,MPI_COMM_WORLD,&status);
  		MPI_Recv(&UPHY(t,(my_rank<sqrt(np) ? 0:1),0),1,colonne,my_rank-1,TAG_UPHY_COLONNE,MPI_COMM_WORLD,&status);

  		// printf("Process %d a reçu HPHY:%lf \n",my_rank,HPHY(t,30,0));
  	}

  	// double test;
  	// scanf("%lf",&test);

  	// MPI_Gather(&HFIL(t,(my_rank<sqrt(np) ? 0:1),(my_rank%(int)sqrt(np)==0) ? 0:1),1,bloc_envoie,&HFIL(t,0,0),1,bloc_envoie,0,MPI_COMM_WORLD);

    // MPI_Gatherv(&HFIL(t,(my_rank<sqrt(np) ? 0:1),(my_rank%(int)sqrt(np)==0) ? 0:1),size_x*size_y,MPI_DOUBLE,&HFIL_global(t,0,0),count,disp,bloc_gather,0,MPI_COMM_WORLD);
  	 // MPI_Gatherv(&HFIL(t,(my_rank<sqrt(np) ? 0:1),(my_rank%(int)sqrt(np)==0) ? 0:1),height_bloc*width_bloc,MPI_DOUBLE,&HFIL_global(t,0,0),count,disp,bloc_gather,0,MPI_COMM_WORLD);
  	  	 // MPI_Gatherv(&HFIL(t,0,0),size_x*size_y,MPI_DOUBLE,&HFIL_global(t,0,0),count,disp,bloc_gather,0,MPI_COMM_WORLD);
  	 // MPI_Gatherv(&HFIL(t,(my_rank<sqrt(np) ? 0:1),(my_rank%(int)sqrt(np)==0) ? 0:1),1,bloc_envoie,&HFIL_global(t,0,0),count,disp,bloc_gather,0,MPI_COMM_WORLD);
  	 // MPI_Gatherv(&HFIL(t,0,0),height_bloc*width_bloc,MPI_DOUBLE,&HFIL_global(t,0,0),count,disp,bloc_gather,0,MPI_COMM_WORLD);
         MPI_Gatherv(&HFIL(t,0,0),height_bloc*width_bloc,MPI_DOUBLE,&HFIL_global(t,0,0),count,disp,bloc_gather,0,MPI_COMM_WORLD);



  	
        if (file_export && my_rank==0) 
          {
  	  export_step(file, t);
          }
        
        if (t == 2) 
          {
  	  dt = svdt;
          }
  }

  if (file_export&&my_rank==0) 
    {
      finalize_export(file);
    }
    // printf("Fin forward process %d\n",my_rank);
}
