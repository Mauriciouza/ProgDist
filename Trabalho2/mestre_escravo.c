#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define RANK_MESTRE 0
#define TAG_OPERACOES 50

#include <sys/types.h>

typedef struct Image
{
  int width;
  int height;
  unsigned char *data;
} Image;

Image *ImageCreate(int width, int height);
Image *ImageRead(char *filename);
void   ImageWrite(Image *image, char *filename);

int    ImageWidth(Image *image);
int    ImageHeight(Image *image);

void   ImageClear(Image *image, unsigned char red, unsigned char green, unsigned char blue);

void   ImageSetPixel(Image *image, int x, int y, int chan, unsigned char val);
unsigned char ImageGetPixel(Image *image, int x, int y, int chan);


int main(int argc, char** argv) {
	

	int comm_sz, my_rank, w, h, x, y, i, j, chan, temp, num, rowSize, offset;
	Image *inImage;
	Image *outImage;
	Image *tempImage;
	unsigned char *data; // dados da imagem
	int *rowSizes; // vetor contendo a divisão de linhas da imagem para cada processo
	int params[2]; // vetor de parametros por processos (w, rowSize)
	unsigned char *section; // seção parcial da imagem para cada processo
	unsigned char *process; // seção de imagem processada para cada processo
	double local_start, local_finish, local_elapsed, elapsed;
	

	// Inicializando o ambiente MPI
	MPI_Init(NULL, NULL);

	// Número de processos
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

	// Rank dos processos
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	// Nome do processador
	int name_len;
	char processor_name[MPI_MAX_PROCESSOR_NAME];
	MPI_Get_processor_name(processor_name, &name_len);
	

	if (my_rank == 0) {
		
		
		// Ler imagens e dimensões
		inImage = ImageRead(argv[1]);
		w = inImage->width;
		h = inImage->height;
		data = inImage->data;
		printf("Imagem: %s, largura: %d, altura: %d\n",argv[1],w,h);
		
		// Tamanho de partições para cada processo
		rowSizes = (int*)malloc(sizeof(int*)*comm_sz);
		num = h / comm_sz; // Número de linhas para cada processo
		temp = h % comm_sz; // Número de linhas restante
		for (x = comm_sz-1; x >= 0; x--) {
			rowSizes[x] = num;
			
			// Em caso de divisão por ímpar, balancear linhas restantes pra cada processo
			if (temp > 0) {
				rowSizes[x]++;
				temp--;
			}
		}

		rowSize = rowSizes[0];
		printf("Processo 0 de %d parametros: largura: %d, linhas: %d\n", comm_sz, w, rowSize);
		
		// Enviando parâmetros para processos > 0
		params[0] = w;
		for (x = 1; x < comm_sz; x++) {
			params[1] = rowSizes[x];
			MPI_Send(params, 2, MPI_INT, x, 0, MPI_COMM_WORLD);
		}
		
		// Enviando porções da imagem para processos > 0
		offset = rowSize;
		for (x = 1; x < comm_sz; x++) {
			temp = w * (rowSizes[x]) * 3;
			section = (unsigned char*)malloc(sizeof(unsigned char*)*temp);
			memcpy(section, data + w*(offset)*3, temp);
			MPI_Send(section, temp, MPI_UNSIGNED_CHAR, x, 0, MPI_COMM_WORLD);
			free(section);
			offset += rowSizes[x];
			
			printf("Processo %d de %d parametros: largura: %d, linhas: %d\n", x, comm_sz, w, rowSizes[x]);
		}
		
		printf("Processando imagem usando %d no(s) ...\n",comm_sz);
		
		// copiar seção para processo 0
		temp = w * (rowSize) * 3;
		section = (unsigned char*)malloc(sizeof(unsigned char*)*temp);
		memcpy(section, data, temp);
		
	} else { // para processos > 0
		// recebendo parametros de processo 0
		MPI_Recv(params, 2, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		w = params[0];
		rowSize = params[1];
		
		// recebendo porção de imagem de processo 0
		temp = w * (rowSize) * 3;
		section = (unsigned char*)malloc(sizeof(unsigned char*)*temp);
		MPI_Recv(section, w * (rowSize) * 3, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
	}
	
	// Processamento da imagem
	
	// tempo de execução
	MPI_Barrier(MPI_COMM_WORLD);
	local_start = MPI_Wtime();
	
	// imagem temporaria para cada processo
	tempImage = ImageCreate(w, rowSize);
	ImageClear(tempImage, 255, 255, 255);
	
	// processando seção de imagem para cada processo 
	if(my_rank==0){
		for (j = 0; j < rowSize; j++) {
			for (i = 0; i < w; i++) {
				temp = 0;
				for (chan = 0; chan < 3; chan++) {
					offset = (j*w+i)*3+chan;
					temp += section[offset];
				}
				temp = temp /3;
				for (chan = 0; chan < 3; chan++) {
					ImageSetPixel(tempImage,i,j,chan,temp);
				}
			}
		}
	}
	if(my_rank==1){
		for (j = 0; j < rowSize; j++) {
			for (i = 0; i < w; i++) {
				temp = 0;
				for (chan = 0; chan < 3; chan++) {
					offset = (j*w+i)*3+chan;
					temp += section[offset];
				}
				temp = temp /3;
				ImageSetPixel(tempImage,i,j,0,temp);
			}
		}
	}
	if(my_rank==2){
		for (j = 0; j < rowSize; j++) {
			for (i = 0; i < w; i++) {
				temp = 0;
				for (chan = 0; chan < 3; chan++) {
					offset = (j*w+i)*3+chan;
					temp += section[offset];
				}
				temp = temp /3;
				ImageSetPixel(tempImage,i,j,1,temp);
			}
		}
	}
	if(my_rank==3){
		for (j = 0; j < rowSize; j++) {
			for (i = 0; i < w; i++) {
				temp = 0;
				for (chan = 0; chan < 3; chan++) {
					offset = (j*w+i)*3+chan;
					temp += section[offset];
				}
				temp = temp /3;
				ImageSetPixel(tempImage,i,j,2,temp);
			}
		}
	}

	free(section);
	
	local_finish = MPI_Wtime();
	local_elapsed = local_finish - local_start;
	MPI_Reduce(&local_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	if (my_rank == 0) {
		printf("Tempo decorrido: %e segundos\n", elapsed);
	}
	
	// enviando imagem de volta para processo 0
	if (my_rank != 0) { // para processos > 0
		
		process = tempImage->data;
		temp = w*rowSize*3;
		MPI_Send(process, temp, MPI_UNSIGNED_CHAR, 0, 0, MPI_COMM_WORLD);
		
	} else { // para processo 0
		
		// criando nova imagem de saída
		outImage = ImageCreate(w, h);
		ImageClear(outImage, 255, 255, 255);
		
		//concatenado imagem
		printf("Processamento completo, concatenando imagem\n");
		offset = 0;
		temp = w*rowSize*3;
		memcpy(outImage->data + w*(offset)*3, tempImage->data, temp);
		printf("Processo 0 de %d adicionando %d linha(s) para imagem processada\n", comm_sz, rowSize);
		
		// recebendo imagem processada em seções e concatenando para imagem final
		offset = rowSize;
		for (x = 1; x < comm_sz; x++) {
			temp = w*rowSizes[x]*3;
			process = (unsigned char*)malloc(sizeof(unsigned char*)*temp);
			MPI_Recv(process, temp, MPI_UNSIGNED_CHAR, x, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			memcpy(outImage->data + w*(offset)*3, process, temp);
			printf("Processo %d de %d adicionando %d linha(s) para imagem processada\n", x, comm_sz, rowSizes[x]);
			free(process);
			offset += rowSizes[x];
		}
		
		// grava imagem processada e libera memória de imagem alocada
		ImageWrite(outImage, argv[2]);
		printf("Imagem processada criada: %s\n", argv[2]);
		free(inImage->data);
		free(outImage->data);
	}

	free(tempImage->data);
	
	MPI_Finalize();
	
	return 0;


	/*int numbers[2];
	int response[2];
	if(world_rank == RANK_MESTRE)
	{
		// Define valores a serem enviados
		numbers[0] = 10;
		numbers[1] = 20;

		// Envia os valores para cada um dos escravos
		for(int i=1; i<world_size; i++)
		{
    		MPI_Send(&numbers[0], 2, MPI_INT, i, TAG_OPERACOES, MPI_COMM_WORLD);
		}
		printf("Processo mestre executando em %s.\n", processor_name);

		// Recebe o resultado processado por cada um dos escravos
		for(int i=1; i<world_size; i++)
		{
    		MPI_Recv(&response[0], 2, MPI_INT, i, TAG_OPERACOES, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    		printf("Resultado de %d: %d.\n", response[0], response[1]);
		}
	}
	else
	{
		MPI_Recv(&numbers[0], 2, MPI_INT, RANK_MESTRE, TAG_OPERACOES, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		printf("Processo escravo %d executando em %s recebeu %d e %d.\n", world_rank, processor_name, numbers[0], numbers[1]);

		// Escravo 1 calcula soma dos valores recebidos
		if(world_rank == 1)
		{
			response[0] = 1;
			response[1] = numbers[0] + numbers[1];
		}

		// Escravo 2 calcula subtracao dos valores recebidos
		else if(world_rank == 2)
		{
			response[0] = 2;
			response[1] = numbers[0] - numbers[1];
		}

		// Escravo 3 calcula produto dos valores recebidos
		else if(world_rank == 3)
		{
			response[0] = 3;
			response[1] = numbers[0] * numbers[1];
		}
		
		// Escravo envia resposta para o mestre
		MPI_Send(&response[0], 2, MPI_INT, RANK_MESTRE, TAG_OPERACOES, MPI_COMM_WORLD);
	}*/

}

static void
die(char *message)
{
  fprintf(stderr, "ppm: %s\n", message);
  exit(1);
}


/* check a dimension (width or height) from the image file for reasonability */

static void
checkDimension(int dim)
{
  if (dim < 1 || dim > 4000) 
    die("file contained unreasonable width or height");
}


/* read a header: verify format and get width and height */

static void
readPPMHeader(FILE *fp, int *width, int *height)
{
  char ch;
  int  maxval;

  if (fscanf(fp, "P%c\n", &ch) != 1 || ch != '6') 
    die("file is not in ppm raw format; cannot read");

  /* skip comments */
  ch = getc(fp);
  while (ch == '#')
    {
      do {
	ch = getc(fp);
      } while (ch != '\n');	/* read to the end of the line */
      ch = getc(fp);            /* thanks, Elliot */
    }

  if (!isdigit(ch)) die("cannot read header information from ppm file");

  ungetc(ch, fp);		/* put that digit back */

  /* read the width, height, and maximum value for a pixel */
  fscanf(fp, "%d%d%d\n", width, height, &maxval);

  if (maxval != 255) die("image is not true-color (24 bit); read failed");
  
  checkDimension(*width);
  checkDimension(*height);
}

Image *
ImageCreate(int width, int height)
{
  Image *image = (Image *) malloc(sizeof(Image));

  if (!image) die("cannot allocate memory for new image");

  image->width  = width;
  image->height = height;
  image->data   = (unsigned char *) malloc(width * height * 3);

  if (!image->data) die("cannot allocate memory for new image");

  return image;
}
  

Image *
ImageRead(char *filename)
{
  int width, height, num, size;
  unsigned  *p;

  Image *image = (Image *) malloc(sizeof(Image));
  FILE  *fp    = fopen(filename, "r");

  if (!image) die("cannot allocate memory for new image");
  if (!fp)    die("cannot open file for reading");

  readPPMHeader(fp, &width, &height);

  size          = width * height * 3;
  image->data   = (unsigned  char*) malloc(size);
  image->width  = width;
  image->height = height;

  if (!image->data) die("cannot allocate memory for new image");

  num = fread((void *) image->data, 1, (size_t) size, fp);

  if (num != size) die("cannot read image data from file");

  fclose(fp);

  return image;
}


void ImageWrite(Image *image, char *filename)
{
  int num;
  int size = image->width * image->height * 3;

  FILE *fp = fopen(filename, "w");

  if (!fp) die("cannot open file for writing");

  fprintf(fp, "P6\n%d %d\n%d\n", image->width, image->height, 255);

  num = fwrite((void *) image->data, 1, (size_t) size, fp);

  if (num != size) die("cannot write image data to file");

  fclose(fp);
}  


int
ImageWidth(Image *image)
{
  return image->width;
}


int
ImageHeight(Image *image)
{
  return image->height;
}


void   
ImageClear(Image *image, unsigned char red, unsigned char green, unsigned char blue)
{
  int i;
  int pix = image->width * image->height;

  unsigned char *data = image->data;

  for (i = 0; i < pix; i++)
    {
      *data++ = red;
      *data++ = green;
      *data++ = blue;
    }
}

void
ImageSetPixel(Image *image, int x, int y, int chan, unsigned char val)
{
  int offset = (y * image->width + x) * 3 + chan;

  image->data[offset] = val;
}


unsigned  char
ImageGetPixel(Image *image, int x, int y, int chan)
{
  int offset = (y * image->width + x) * 3 + chan;

  return image->data[offset];
}
