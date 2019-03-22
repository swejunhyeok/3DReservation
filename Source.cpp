#include <iostream>
#include "My_Mesh.h"
#include "My_Mtl.h"
#include <float.h>
#include <cstdlib>
#include <windows.h>
#include "Vec3.h"
#include "My_Color.h"
#include "Camera.h"
#include <opencv2\opencv.hpp>

#pragma warning(disable: 4996)

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define StadiumRedering 0
#define SeatRendering 1

#define FirstCapture 1
#define SecondCapture 2
#define FourthCapture 4


Camera FirstCamera(LookAtCamera, 0.0, -5.0, 10.0, 0.0, -4.0, -80.0, 0.0);
Camera SecondCamera(MatrixCamera, 0.0, 0.0, -12.0, 90.0, 0.5, 0.0);
Camera ThirdCamera(MatrixCamera, 0.0, 0.0, -5.0, 0.0, 0.001, 0.0);
Camera FourthCamera(MatrixCamera, -1.0, -0.25, -3.0, 0.0, 90.0, 0.0);
bool FourthMove[4] = {false};
bool FourthRota[4] = {false};

float delta = 0.1, alpha = 0.001;
float Fdelta = 0.1, Falpha = 0.001, FRota = 1.0;
float FIncRate = 0.7, FDecRate = 0.4;

int lastx=0;
int lasty=0;
unsigned char Buttons[3] = {0};

int FirstPic = 0;
int FourthPic = 0;
int SecondPic = 0;

int SeatColor = 0;

GLuint textures[3];

// Mesh structure
My_Mesh m_Mesh[2];

int MeshCnt = 0;
vector<My_Mtl> m_Mtl;
vector<My_Color> m_Color;

bool Clickflag = false;
bool Colorflag = true;
bool CameraAnim = false;
bool IncAnim = true;
bool Returnflag = false;
bool Captureflag = true;
bool FirstCaptureflag = false;
bool FourthCaptureflag = false;

float Width ,Height;

void TransformationToCenter(double scale, double &min_x, double &max_x, double &min_y, double &max_y, double &min_z, double &max_z)
{
	double longestAxis = max(max(fabs(max_x-min_x),fabs(max_y-min_y)),fabs(max_z-min_z));
	Vec3<double> vMax(max_x, max_y, max_z);
	Vec3<double> vMin(min_x, min_y, min_z);	
	Vec3<double> center = (vMax+vMin)/2.0; 
	Vec3<double> position;
	Vec3<double> vector;
	min_x = min_y = min_z = DBL_MAX;
	max_x = max_y = max_z = DBL_MIN;	
	Vec3<double> Update_Center(0.0, 0.0, 0.0);

	for(int i = 0; i < (int)m_Mesh[MeshCnt].m_Vertices.size(); i++) {
		My_Vertex &vertex = m_Mesh[MeshCnt].m_Vertices[i];
		position.Set(vertex.x(), vertex.y(), vertex.z());
		vector = position-center;
		vector /= longestAxis;
		vector *= scale;
		position = Update_Center;
		position += vector;
		for(int j = 0; j < 3; j++) {
			vertex.m_Position[j] = position[j];
		}
		double x = vertex.m_Position[0];
		double y = vertex.m_Position[1];
		double z = vertex.m_Position[2];
		if(min_x > (double)x)	min_x = (double)x;
		if(max_x < (double)x)	max_x = (double)x;
		if(min_y > (double)y)	min_y = (double)y;
		if(max_y < (double)y)	max_y = (double)y;
		if(min_z > (double)z)	min_z = (double)z;
		if(max_z < (double)z)	max_z = (double)z;
	}
}

void Load_texture(char *filename, GLuint &texture_id)
{
	//glGenTextures(12, textures);
	//Texture 수가 여럿일 때는 버퍼 갯수 조정
	//glGenTextures(3, textures);

	FILE *fp = fopen(filename, "rb");
	if(!fp) {
		printf("ERROR : No %s.\n fail to bind %d\n", filename, texture_id);  
		return;
	}
	int width, height, channel;
	unsigned char *image = stbi_load_from_file(fp, &width, &height, &channel, 4);
	fclose(fp);

	//Texture 수가 여럿일 경우, 각 Texture 변수에 대해 각각 선언해야 함
	//bind
	glBindTexture(GL_TEXTURE_2D, texture_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, GL_MODULATE);
}

void OpenMtl(char *filename){
	FILE *fp;

	char buffer[256] = {0};
	char map[256] = {0};
	char mtlName[256] = {0};
	char l_char;
	double Kd[3], Ka[3], Ke[3], Ks[3];
	int d, Ns, illum;
	bool flag = 0;

	fopen_s(&fp, filename, "r");
	while (!feof(fp)) //Loop to scan the whole file 
	{
		fscanf(fp,"%c",&l_char);
		if(l_char=='\n')//read char if'/n' -skip to next and read   
			fscanf(fp,"%c",&l_char);
		switch (l_char) //parse
		{   
		default: fgets(buffer,256,fp);
			break;
		case 'K':   //a vertex or a normal or a text co-ord
			fscanf(fp,"%c",&l_char);
			switch (l_char)  
			{
			case 'd':
				fscanf(fp, "%lf %lf %lf", &Kd[0], &Kd[1], &Kd[2]);
				break;
			case 'a':
				fscanf(fp, "%lf %lf %lf", &Ka[0], &Ka[1], &Ka[2]);
				break;
			case 's':
				fscanf(fp, "%lf %lf %lf", &Ks[0], &Ks[1], &Ks[2]);
				break;
			case 'e':
				fscanf(fp, "%lf %lf %lf", &Ke[0], &Ke[1], &Ke[2]);
				break;
			}  //end switch
			break;
		case 'd': //a face read next assume format is -> f 1/1/1 2/2/2 3/3/3
			fscanf(fp, "%d", &d);
			break;
		case 'N' :
			fscanf(fp,"%c",&l_char);
			if(l_char == 's')
				fscanf(fp, "%d", &Ns);
			break;
		case 'i':
			fgets(buffer,256,fp);
			fscanf(fp, "%d", &illum);
			break;
		case 'm' :
			fscanf(fp,"%s ",buffer);
			fgets(map, 256, fp);
			map[strlen(map) - 1] = '\0';
			break;
		case 'n' :
			if(flag){
				m_Mtl.push_back(My_Mtl(mtlName, Kd[0], Kd[1], Kd[2], Ka[0], Ka[1], Ka[2], Ks[0], Ks[1], Ks[2], Ke[0], Ke[1], Ke[2], d, Ns, illum, map));
			}
			for(int i = 0; i < 255; i++)
				map[i] = 0;
			fscanf(fp, "%s ", &buffer);
			fgets(mtlName, 256, fp);
			mtlName[strlen(mtlName) - 1] = '\0';
			flag = 1;
			break;
		} //end switch
	}
	m_Mtl.push_back(My_Mtl(mtlName, Kd[0], Kd[1], Kd[2], Ka[0], Ka[1], Ka[2], Ks[0], Ks[1], Ks[2], Ke[0], Ke[1], Ke[2], d, Ns, illum, map));
	fclose(fp);
	//printf("Number of Mtl : %d\n", m_Mtl.size());
}

void OpenOBJ(char *filename)
{
	FILE *fp;

	char buffer[256] = {0};
	char mtlName[256] = "0";
	char mtlFile[256] = "0";
	char l_char;
	double pos[3];
	double nor[3];
	double vt[2];
	int index[3];
	int tex[3];
	int norInd[3];
	double minX, minY, minZ, maxX, maxY, maxZ;
	minX = minY = minZ = DBL_MAX;
	maxX = maxY = maxZ = DBL_MIN;
	My_Vertex vertex(0, 100, 0);

	fopen_s(&fp, filename, "r");
	while (!feof(fp)) //Loop to scan the whole file 
	{
		fscanf(fp,"%c",&l_char);
		if(l_char=='\n')//read char if'/n' -skip to next and read   
			fscanf(fp,"%c",&l_char);
		switch (l_char) //parse
		{   
		default: fgets(buffer,256,fp);
			break;
		case 'v':   //a vertex or a normal or a text co-ord
			fscanf(fp,"%c",&l_char);
			switch (l_char)  
			{
			case ' ':   //a vertex -expect and so read 3 floats next
				fscanf(fp, "%lf %lf %lf", &pos[0], &pos[1], &pos[2]);
				if(minX > pos[0])	minX = pos[0];
				if(minY > pos[1])	minY = pos[1];
				if(minZ > pos[2])	minZ = pos[2];
				if(maxX < pos[0])	maxX = pos[0];
				if(maxY < pos[1])	maxY = pos[1];
				if(maxZ < pos[2])	maxZ = pos[2];
				m_Mesh[MeshCnt].m_Vertices.push_back(My_Vertex(pos[0], pos[1], pos[2]));
				break;
			case 'n': //a normal -expect and so read 3 floats next
				fscanf(fp, "%lf %lf %lf", &nor[0], &nor[1], &nor[2]);
				m_Mesh[MeshCnt].m_Normals.push_back(My_Vertex(nor[0], nor[1], nor[2]));
				break;
			case 't': //a texture map coord-expect and so read 2 floats next
				fscanf(fp, "%lf %lf", &vt[0], &vt[1]);
				m_Mesh[MeshCnt].m_Tex.push_back(My_Texture(vt[0], 1-vt[1], 0.0));
				break;
			}  //end switch
			break;
		case 'f': //a face read next assume format is -> f 1/1/1 2/2/2 3/3/3
			fscanf(fp, "%d/", &index[0]);
			fscanf(fp, "%c", &l_char);
			if(l_char == '/'){
				fscanf(fp, "%d %d//%d %d//%d",&norInd[0], &index[1], &norInd[1], &index[2], &norInd[2]);
				m_Mesh[MeshCnt].m_Faces.push_back(My_Face(mtlName, index[0]-1, index[1]-1, index[2]-1, norInd[0]-1, norInd[1]-1, norInd[2]-1, true));
			}	
			else{
				fseek(fp, -1, SEEK_CUR);
				fscanf(fp, "%d/%d %d/%d/%d %d/%d/%d", &tex[0], &norInd[0], &index[1], &tex[1], &norInd[1], &index[2], &tex[2], &norInd[2]);
				m_Mesh[MeshCnt].m_Faces.push_back(My_Face(mtlName, index[0]-1, index[1]-1, index[2]-1, tex[0]-1, tex[1]-1, tex[2]-1, norInd[0]-1, norInd[1]-1, norInd[2]-1));
			}
			break;
		case 'm' :
			char Mtltemp[256];
			fscanf(fp, "%s ", Mtltemp);
			fgets(mtlFile, 256, fp);
			mtlFile[strlen(mtlFile) - 1] = '\0';
			break;
		case 'u':
			char temp[256];
			fscanf(fp, "%s ", temp);
			fgets(mtlName, 256, fp);
			mtlName[strlen(mtlName) - 1] = '\0';
			break;
		} //end switch
	}
	fclose(fp);

	for(int Index = 0; Index < m_Mesh[MeshCnt].m_Vertices.size(); Index++){
		m_Color.push_back(My_Color(&m_Mesh[MeshCnt].m_Vertices[Index]));
	}

	//printf("Number of Vertex : %d\n", m_Mesh[MeshCnt].m_Vertices.size());
	//printf("Number of Color : %d\n", m_Color.size());
	//printf("Number of Normal : %d\n", m_Mesh[MeshCnt].m_Normals.size());
	//printf("Number of Texture : %d\n", m_Mesh[MeshCnt].m_Tex.size());
	//printf("Number of Face : %d\n", m_Mesh[MeshCnt].m_Faces.size());

	if(MeshCnt == 1)
	TransformationToCenter(5, minX, maxX, minY, maxY, minZ, maxZ);


	if(mtlFile[0] != '0'){
		OpenMtl(mtlFile);
	}
	MeshCnt++;
}

void LoadOBJ(int MeshNum){
	int VertexNum = m_Mesh[MeshNum].m_Vertices.size();
	int NormalNum = m_Mesh[MeshNum].m_Normals.size();
	int TextureNum = m_Mesh[MeshNum].m_Tex.size();
	int FaceNum = m_Mesh[MeshNum].m_Faces.size();
	My_Vertex vertex(0, 100, 0);

	for(int i = 0; i < VertexNum; i++){
		//printf("Angle : %lf\n", vertex.computeAngle(m_Mesh[MeshNum].m_Vertices[i]));
		printf("v : %lf %lf %lf \n",m_Mesh[MeshNum].m_Vertices[i].x(), m_Mesh[MeshNum].m_Vertices[i].y(), m_Mesh[MeshNum].m_Vertices[i].z());
	}
	for(int i = 0; i < NormalNum; i++){
		printf("vn : %lf %lf %lf \n",m_Mesh[MeshNum].m_Normals[i].x(), m_Mesh[MeshNum].m_Normals[i].y(), m_Mesh[MeshNum].m_Normals[i].z());
	}
	for(int i = 0; i < TextureNum; i++){
		printf("vt : %lf %lf %lf \n",m_Mesh[MeshNum].m_Tex[i].x(), m_Mesh[MeshNum].m_Tex[i].y(), m_Mesh[MeshNum].m_Tex[i].z());
	}
	for(int i = 0; i < TextureNum; i++){
		printf("Mtlname = %s, f : %d %d %d \n",m_Mesh[MeshNum].m_Faces[i].m_Mtl, m_Mesh[MeshNum].m_Faces[i].m_Index[0], m_Mesh[MeshNum].m_Faces[i].m_Index[1], m_Mesh[MeshNum].m_Faces[i].m_Index[2]);
	}
}

void LoadMtl(){
	int NumberOfMtl = m_Mtl.size();
	for(int i = 0; i < NumberOfMtl; i++){
		printf("newmtl %s\n", m_Mtl[i].Name);
		printf("Kd : %lf %lf %lf\n", m_Mtl[i].kd[0], m_Mtl[i].kd[1], m_Mtl[i].kd[2]);
		printf("Ka : %lf %lf %lf\n", m_Mtl[i].ka[0], m_Mtl[i].ka[1], m_Mtl[i].ka[2]);
		printf("Ks : %lf %lf %lf\n", m_Mtl[i].ks[0], m_Mtl[i].ks[1], m_Mtl[i].ks[2]);
		printf("Ke : %lf %lf %lf\n", m_Mtl[i].ke[0], m_Mtl[i].ke[1], m_Mtl[i].ke[2]);
		printf("d : %d\n", m_Mtl[i].d);
		printf("Ns : %d\n", m_Mtl[i].Ns);
		printf("illum : %d\n", m_Mtl[i].illum);
		if(m_Mtl[i].Map[0] != 0)
			printf("Map %s\n", m_Mtl[i].Map);
	}
}

int findMaterialIndex(char *name)
{
	int numberMtl = m_Mtl.size();
	for (int i=0; i < numberMtl; ++i) {
		if (!strcmp (name, m_Mtl[i].Name)) {
			return i;
		}
	}
	return -1;
}


void Caputre(char *name, int Case){
	BITMAPFILEHEADER bmpfile; // 비트맵파일헤더
	BITMAPINFOHEADER bmpinfo; // 비트맵정보헤더

	char str[10];

	unsigned char *pixels = new unsigned char[sizeof(unsigned char)*Width*Height * 3]; //unsigned char = BYTE 0-255

	switch(Case){
	case FirstCapture :
		ultoa(FirstPic++, str, 10);
		break;
	case SecondCapture:
		ultoa(SecondPic, str, 10);
		break;
	case FourthCapture:
		ultoa(FourthPic++, str, 10);
		break;
	}
	strcat(name, str);
	strcat(name, ".bmp");
	//FILE *file = fopen(name, "wb");
	FILE *file = fopen(name, "wb");
	glReadPixels(0, 0, Width, Height, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	memset(&bmpfile, 0, sizeof(bmpfile)); // 14바이트 0 값.
	memset(&bmpinfo, 0, sizeof(bmpinfo)); // 
	bmpfile.bfType = 'MB';
	bmpfile.bfSize = sizeof(bmpfile) + sizeof(bmpinfo) + Width*Height * 3;
	bmpfile.bfOffBits = sizeof(bmpfile) + sizeof(bmpinfo);

	bmpinfo.biSize = sizeof(bmpinfo); // 구조체크기
	bmpinfo.biWidth = Width; // 이미지 가로
	bmpinfo.biHeight = Height; // 이미지 세로
	bmpinfo.biPlanes = 1; // 플레인수
	bmpinfo.biBitCount = 24; // 비트수 
	bmpinfo.biSizeImage = Width*Height * 3; // 이미지의크기

	fwrite(&bmpfile, sizeof(bmpfile), 1, file);
	fwrite(&bmpinfo, sizeof(bmpinfo), 1, file);
	fwrite(pixels, sizeof(unsigned char), Height*Width * 3, file);

	fclose(file);
	free(pixels);
}

void MakeVideo(int mode){
	CvSize size = cvSize(Width / 2, Height / 2);
	CvVideoWriter *writer;
	int PicNum = 0;
	
	if(mode == FirstCapture){
		PicNum = --FirstPic;
		writer = cvCreateVideoWriter("First.avi", 0, 30, size, 1);
	}
	else if(mode == FourthCapture){
		PicNum = --FourthPic;
		writer = cvCreateVideoWriter("Fourth.avi", 0, 30, size, 1);
	}

	if(writer == NULL){
		printf("No Codec!");
	}

	for(int Index = 0; Index < PicNum; Index++){
		char str[10];
		char name[100] = "";
		if(mode == FirstCapture)
			strcpy(name, "First\\First");
		else if(mode == FourthCapture)
			strcpy(name, "Fourth\\Fourth");
			
		IplImage *img = cvCreateImage(size, 8, 3);
		ultoa(Index, str, 10);
		strcat(name, str);
		strcat(name, ".bmp");
		IplImage *src = cvLoadImage(name);
		for(int x = 0; x < img->width; x++){
			for(int y = 0; y < img->height; y++){
				CvScalar c;
				if(mode == FirstCapture)
					c = cvGet2D(src , y, x + (Width / 2));
				else if(mode == FourthCapture)
					c = cvGet2D(src , y + (Height / 2), x + (Width / 2));
				CvScalar final;
				final.val[0] = c.val[2];
				final.val[1] = c.val[1];
				final.val[2] = c.val[0];
				cvSet2D(img, y, x, final);
			}
		}
		cvWriteFrame(writer, img);
	}
	cvReleaseVideoWriter(&writer);
}

void RenderingMesh(int MeshNum)
{
	bool flag = true;
	glColor3f(0, 0, 0);
	glPushMatrix();
	int numberFaces = m_Mesh[MeshNum].m_Faces.size();

	//Texture coordinate 저장하는 코드 필요
	for(int i = 0; i < numberFaces; i++) {
		My_Face f = m_Mesh[MeshNum].m_Faces[i];		
		My_Vertex v[3];
		My_Texture uv[3];	//여기가 texture coordinate를 위한 변수
		My_Vertex n[3];
		My_Color c[3];

		int Index = findMaterialIndex(f.m_Mtl);
		if(m_Mtl[Index].Map[0] != 0){
			glEnable(GL_TEXTURE_2D);
			if(!strcmp(m_Mtl[Index].Map, "37_field.jpg")){
				glBindTexture(GL_TEXTURE_2D, textures[0]);
				Colorflag = false;
				//printf("Call : %d\n", textures[0]);
			}
			if(!strcmp(m_Mtl[Index].Map, "37_main.jpg")){
				Colorflag = true;
				glBindTexture(GL_TEXTURE_2D, textures[1]);
			}
			if(!strcmp(m_Mtl[Index].Map, "37_round.jpg")){
				Colorflag = false;
				glBindTexture(GL_TEXTURE_2D, textures[2]);
			}
		}
		else{
			glDisable(GL_TEXTURE_2D);
		}

		for(int j = 0; j < 3; j++) {
			v[j] = m_Mesh[MeshNum].m_Vertices[f.m_Index[j]];

			c[j] = m_Color[f.m_Index[j]];

			if(f.m_TexIdx[j] == -1)
				flag = false;
			else{
				flag = true;
				uv[j] = m_Mesh[MeshNum].m_Tex[f.m_TexIdx[j]];
			}
			n[j] = m_Mesh[MeshNum].m_Normals[f.m_NormalIdx[j]];
		}
		glBegin(GL_POLYGON);
		for(int j = 0; j < 3; j++) {
			glNormal3f((GLfloat)n[j].x(), n[j].y(), n[j].z());

			if(flag){
				glTexCoord2f((GLfloat)uv[j].x(), (GLfloat)uv[j].y());
				glColor3f(1.0f, 1.0f, 1.0f);
			}
			if(Colorflag){
				glColorMaterial(GL_FRONT, GL_DIFFUSE);
				if(!strcmp(c[j].Color, "R"))
					glColor3f(1.0f, 0.0f, 0.0f);
				else if(!strcmp(c[j].Color, "G"))
					glColor3f(0.0f, 1.0f, 0.0f);
				else if(!strcmp(c[j].Color, "B"))
					glColor3f(0.0f, 0.0f, 1.0f);
				else if(!strcmp(c[j].Color, "RG"))
					glColor3f(1.0f, 1.0f, 0.0f);
				else if(!strcmp(c[j].Color, "RB"))
					glColor3f(1.0f, 0.0f, 1.0f);
				else if(!strcmp(c[j].Color, "GB"))
					glColor3f(0.0f, 1.0f, 1.0f);
			}
			glVertex3f((GLfloat)v[j].x(), (GLfloat)v[j].y(), (GLfloat)v[j].z());
		}
		glEnd();
	}
	glPopMatrix();

	//Texture 사용이 끝났으면 disable 해줘야 함
	glDisable(GL_TEXTURE_2D);
}

void RenderingSeat(){
	bool flag = true;
	glPushMatrix();
	int numberFaces = m_Mesh[SeatRendering].m_Faces.size();

	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	if(SeatColor == 0)
		glColor3f(1.0f, 1.0f, 1.0f);
	else if(SeatColor == 1)
		glColor3f(1.0f, 1.0f, 0.0f);
	else if(SeatColor == 2)
		glColor3f(1.0f, 0.0f, 1.0f);
	else if(SeatColor == 3)
		glColor3f(0.0f, 1.0f, 1.0f);
	else if(SeatColor == 4)
		glColor3f(1.0f, 0.0f, 0.0f);
	else if(SeatColor == 5)
		glColor3f(0.0f, 1.0f, 0.0f);
	else if(SeatColor == 6)
		glColor3f(0.0f, 0.0f, 1.0f);

	glDisable(GL_TEXTURE_2D);

	//Texture coordinate 저장하는 코드 필요
	for(int i = 0; i < numberFaces; i++) {
		My_Face f = m_Mesh[SeatRendering].m_Faces[i];		
		My_Vertex v[3];
		My_Vertex n[3];

		for(int j = 0; j < 3; j++) {
			v[j] = m_Mesh[SeatRendering].m_Vertices[f.m_Index[j]];
			n[j] = m_Mesh[SeatRendering].m_Normals[f.m_NormalIdx[j]];
		}
		glBegin(GL_POLYGON);
		for(int j = 0; j < 3; j++) {
			glNormal3f((GLfloat)n[j].x(), n[j].y(), n[j].z());
			glVertex3f((GLfloat)v[j].x(), (GLfloat)v[j].y(), (GLfloat)v[j].z());
		}
		glEnd();
	}
	glPopMatrix();
}

void RederingOfFirst(void){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45,(float)Width/Height,0.1,100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//printf("%f %f %f  ///  %f %f %f \n", FirstCamera.Position[0], FirstCamera.Position[1], FirstCamera.Position[2], FirstCamera.Direction[0], FirstCamera.Direction[1], FirstCamera.Direction[2]);
	gluLookAt(FirstCamera.Position[0], FirstCamera.Position[1], FirstCamera.Position[2],
		FirstCamera.Direction[0], FirstCamera.Direction[1], FirstCamera.Direction[2],
		0.0f, 1.0f,  0.0f);
	// Init lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glShadeModel(GL_SMOOTH);


	RenderingMesh(StadiumRedering);	// Draw mesh
	if(FirstCaptureflag){
		char str[100] =  "First\\First";
		Caputre(str, FirstCapture);
	}
	glDisable(GL_LIGHTING);
}

void RederingOfSecond(void){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45,(float)Width/Height,0.1,100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(SecondCamera.Translate[0], 0, 0);
	glTranslatef(0, SecondCamera.Translate[1], 0);
	glTranslatef(0, 0, SecondCamera.Translate[2]);
	glRotatef(SecondCamera.Rotation[0],1,0,0);
	glRotatef(SecondCamera.Rotation[1],0,1,0);
	glRotatef(SecondCamera.Rotation[2],0,0,1);

	// Init lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glShadeModel(GL_SMOOTH);

	RenderingMesh(StadiumRedering);	// Draw mesh
	/*
	if(Captureflag){
		char str[100] = "Second";
		Caputre(str, SecondCapture);
		Captureflag = false;
	}*/
	char str[100] = "Second";
	Caputre(str, SecondCapture);
	glDisable(GL_LIGHTING);
}

void RederingOfThird(void){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45,(float)Width/Height,0.1,100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glTranslatef(ThirdCamera.Translate[0], 0, 0);
	glTranslatef(0, ThirdCamera.Translate[1], 0);
	glTranslatef(0, 0, ThirdCamera.Translate[2]);
	glRotatef(ThirdCamera.Rotation[0],1,0,0);
	glRotatef(ThirdCamera.Rotation[1],0,1,0);
	glRotatef(ThirdCamera.Rotation[2],0,0,1);

	// Init lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glShadeModel(GL_SMOOTH);
	if(Clickflag)
		RenderingSeat();	// Draw mesh

	glDisable(GL_LIGHTING);
}

void RederingOfFourthly(void){
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45,(float)Width/Height,0.1,100);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//printf("zoom : %f, tx : %f, ty : %f, rotx : %f, roty : %f \n", FourthCamera.Translate[2], FourthCamera.Translate[0], FourthCamera.Translate[1], FourthCamera.Rotation[0], FourthCamera.Rotation[1]);

	glTranslatef(FourthCamera.Translate[0], 0, 0);
	glTranslatef(0, FourthCamera.Translate[1], 0);
	glTranslatef(0, 0, FourthCamera.Translate[2]);

	glRotatef(FourthCamera.Rotation[0], 1, 0, 0);
	glRotatef(FourthCamera.Rotation[1], 0, 1, 0);
	glRotatef(FourthCamera.Rotation[2], 0, 0, 1);


	// Init lighting
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glShadeModel(GL_SMOOTH);

	RenderingMesh(StadiumRedering);	// Draw mesh
	char str[100] =  "Fourth\\Fourth";
	if(FourthCaptureflag){
		Caputre(str, FourthCapture);
	}
	glDisable(GL_LIGHTING);

}


void SplitDisplay(void)
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	// 제 3사분면
	glViewport(0, 0, Width / 2, Height / 2) ;
	RederingOfThird();

	//제 4사분면
	glViewport(Width / 2, 0, Width / 2, Height / 2) ;
	RederingOfFourthly();

	//제 2사분면
	glViewport(0, Height / 2, Width / 2, Height / 2) ;
	RederingOfSecond();

	//제 1사분면
	glViewport(Width / 2, Height / 2, Width / 2, Height / 2) ;
	RederingOfFirst();

	glutSwapBuffers();
}


void SelectColor(int x, int y){
	IplImage *img = cvLoadImage("Second0.bmp");

	CvScalar c = cvGet2D(img ,y, x);
	if(c.val[0] > 200 && c.val[1] > 200 && c.val[2] > 200)
		SeatColor = 0; // white
	else if(c.val[0] > 200 && c.val[1] > 200)
		SeatColor = 1; // RG
	else if(c.val[0] > 200 && c.val[2] > 200)
		SeatColor = 2; // RB
	else if(c.val[1] > 200 && c.val[2] > 200)
		SeatColor = 3; // GB
	else if(c.val[0] > 200)
		SeatColor = 4; // R
	else if(c.val[1] > 200)
		SeatColor = 5; // G
	else if(c.val[2] > 200)
		SeatColor = 6; // B
}

void Motion(int x,int y)
{
	int diffx=x-lastx;
	int diffy=y-lasty;
	lastx=x;
	lasty=y;

	if(Buttons[0] && Buttons[2]) {
		SecondCamera.Translate[2] -= (float) 0.05f * diffx;
	} else if(Buttons[0]) {
		SecondCamera.Rotation[0] += (float) 0.5f * diffy;
		SecondCamera.Rotation[1] += (float) 0.5f * diffx;		
	} else if(Buttons[1]) {
		SecondCamera.Translate[0] += (float) 0.05f * diffx;
		SecondCamera.Translate[1] -= (float) 0.05f * diffy;
	}
	glutPostRedisplay();
}

void Mouse(int button,int state,int x,int y)
{
	if(x < 400 && y < 300){
		lastx=x;
		lasty=y;
		switch(button)
		{
		case GLUT_LEFT_BUTTON:
			Buttons[0] = ((GLUT_DOWN==state)?1:0);
			break;
		case GLUT_MIDDLE_BUTTON:
			Buttons[1] = ((GLUT_DOWN==state)?1:0);
			break;
		case GLUT_RIGHT_BUTTON:
			Clickflag = true;
			SelectColor(x, y);
			Buttons[2] = ((GLUT_DOWN==state)?1:0);
			break;
		default:
			break;
		}
		glutPostRedisplay();
	}
	else if(x > 400 && y > 300){
		FourthCaptureflag = true;
		FourthPic = 0;
		FourthMove[0] = true;
	}
	else if(x > 400 && y < 300){
		if(button == GLUT_LEFT_BUTTON){
			CameraAnim = true;
			FirstPic = 0;
			FirstCaptureflag = true;
		}
	}
	glutPostRedisplay();
}

void Keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
	case 27:	// 'Esc'
		exit(0);
		break;
	}
}


void Init(void) 
{
	glEnable(GL_DEPTH_TEST);
	glDepthRange(0.0, 1.0);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	//Load Texture
	glGenTextures(3, textures);

	Load_texture("37_field.jpg", textures[0]);
	//Load_texture("37_main.jpg", textures[1]);
	Load_texture("37_round.jpg", textures[2]);
}

void SplitReshape(int w, int h)
{
	if(h==0) {
		h = 1;
	}
	Width = w;
	Height = h;
}

void FirstCameraAnimation() {
	if(CameraAnim){
		//Y값이 감소 할때는 속도가 증가
		//Y값이 증가 할때는 속도가 감소
		//첫점 (-5, 10, 0)
		//중간점 (0, 5, 0)	// Y값이 -5 증가하면 X값은 +5 증가
		//끝점 (5, 10, 0)
		//printf(" FirstCameraPX: %f, FirstCameraPY : %f\n",FirstCamera.Position[0] ,FirstCamera.Position[1]);
		//카메라의 Direction = 다음 카메라 이동 정점
		if(IncAnim){
			alpha += alpha * 0.6;

			FirstCamera.Position[0] += (delta + alpha);
			FirstCamera.Position[1] -= (delta + alpha);

			FirstCamera.Direction[0] = 1.0f + FirstCamera.Position[0];
			FirstCamera.Direction[1] = FirstCamera.Direction[1] - 90.0;
		}
		else{
			alpha -= alpha * 0.6;

			FirstCamera.Position[0] += (delta + alpha);
			FirstCamera.Position[1] += (delta + alpha);

			FirstCamera.Direction[0] = 1.0f + FirstCamera.Position[0];
			FirstCamera.Direction[1] = FirstCamera.Direction[1] - 90.0;
		}
		if(FirstCamera.Position[1] < 5){
			FirstCamera.Position[0] = 0;
			FirstCamera.Position[1] = 5;
			IncAnim = false;
		}
		if(FirstCamera.Position[1] > 10 && !IncAnim){
			FirstCamera.Position[0] = 5;
			FirstCamera.Position[1] = 10;
			IncAnim = true;
			CameraAnim = false;
			Returnflag = true;
			alpha = 0.001;
		}
		glutPostRedisplay();
	}
	if(Returnflag){
		//printf(" FirstCameraPX: %f, FirstCameraPY : %f\n",FirstCamera.Position[0] ,FirstCamera.Position[1]);
		if(IncAnim){
			alpha += alpha * 0.6;

			FirstCamera.Position[0] -= (delta + alpha);
			FirstCamera.Position[1] -= (delta + alpha);

			FirstCamera.Direction[0] = 1.0f + FirstCamera.Position[0];
			FirstCamera.Direction[1] = FirstCamera.Direction[1] - 90.0;
		}
		else{
			alpha -= alpha * 0.6;

			FirstCamera.Position[0] -= (delta + alpha);
			FirstCamera.Position[1] += (delta + alpha);

			FirstCamera.Direction[0] = 1.0f + FirstCamera.Position[0];
			FirstCamera.Direction[1] = FirstCamera.Direction[1] - 90.0;
		}
		if(FirstCamera.Position[1] < 5){
			FirstCamera.Position[0] = 0;
			FirstCamera.Position[1] = 5;
			IncAnim = false;
		}
		if(FirstCamera.Position[1] > 10 && !IncAnim){
			alpha = 0.001;
			IncAnim = true;
			CameraAnim = false;
			Returnflag = false;
			FirstCaptureflag = false;
			FirstCamera.Position[0] = -5.0;
			FirstCamera.Position[1] = 10.0;
			FirstCamera.Direction[0] = 1.0f + FirstCamera.Position[0];
			FirstCamera.Direction[1] = FirstCamera.Direction[1] - 90.0;
			MakeVideo(FirstCapture);
		}
		glutPostRedisplay();
	}
}

void FourthCameraAnimation(){
	if(FourthMove[0]){
		if(FourthCamera.Translate[2] < -0.75)
			Falpha += Falpha * FIncRate;
		else
			Falpha -= Falpha * FDecRate;
		FourthCamera.Translate[2] += Fdelta + Falpha;
		glutPostRedisplay();
		if(FourthCamera.Translate[2] > 2.0){
			FourthCamera.Translate[2] = 2.0;
			Falpha = 0.001;
			FourthMove[0] = false;
			FourthRota[0] = true;
			glutPostRedisplay();
		}
	}
	else if(FourthRota[0]){
		FourthCamera.Translate[2] -= 0.116;
		FourthCamera.Rotation[1] -= 3;
		FourthCamera.Translate[0] -= 0.05;
		glutPostRedisplay();
		if(FourthCamera.Rotation[1] < 0.0){
			FourthCamera.Translate[0] = -2.5;
			FourthCamera.Translate[2] = -1.5;
			FourthCamera.Rotation[1] = 0.0;
			Falpha = 0.001;
			FourthRota[0] = false;
			FourthMove[1] = true;
			glutPostRedisplay();
		}
	}
	else if(FourthMove[1]){
		if(FourthCamera.Translate[2] < -0.25)
			Falpha += Falpha * FIncRate;
		else
			Falpha -= Falpha * FDecRate;
		FourthCamera.Translate[2] += Fdelta + Falpha;
		glutPostRedisplay();
		if(FourthCamera.Translate[2] > 1){
			FourthCamera.Translate[2] = 1;
			Falpha = 0.001;
			FourthMove[1] = false;
			FourthRota[1] = true;
			glutPostRedisplay();
		}
	}
	else if(FourthRota[1]){
		FourthCamera.Translate[0] += 0.033;
		FourthCamera.Translate[2] -= 0.13;
		FourthCamera.Rotation[1] -= 3.0;
		glutPostRedisplay();
		if(FourthCamera.Rotation[1] < -90.0){
			FourthCamera.Translate[0] = -1.5;
			FourthCamera.Translate[2] = -3.0;
			FourthCamera.Rotation[1] = -90.0;
			Falpha = 0.001;
			FourthRota[1] = false;
			FourthMove[2] = true;
			glutPostRedisplay();
		}
	}
	else if(FourthMove[2]){
		if(FourthCamera.Translate[2] < -0.25)
			Falpha += Falpha * FIncRate;
		else
			Falpha -= Falpha * FDecRate;
		FourthCamera.Translate[2] += Fdelta + Falpha;
		glutPostRedisplay();
		if(FourthCamera.Translate[2] > 2.5){
			FourthCamera.Translate[2] = 2.5;
			Falpha = 0.001;
			FourthMove[2] = false;
			FourthRota[2] = true;
			glutPostRedisplay();
		}
	}
	else if(FourthRota[2]){
		FourthCamera.Translate[0] -= 0.033;
		FourthCamera.Translate[2] -= 0.13;
		FourthCamera.Rotation[1] -= 3.0;
		glutPostRedisplay();
		if(FourthCamera.Rotation[1] < -180.0){
			FourthCamera.Translate[0] = -2.5;
			FourthCamera.Translate[2] = -1.5;
			FourthCamera.Rotation[1] = -180.0;
			Falpha = 0.001;
			FourthRota[2] = false;
			FourthMove[3] = true;
			glutPostRedisplay();
		}
	}
	else if(FourthMove[3]){
		if(FourthCamera.Translate[2] < -0.25)
			Falpha += Falpha * FIncRate;
		else
			Falpha -= Falpha * FDecRate;
		FourthCamera.Translate[2] += Fdelta + Falpha;
		glutPostRedisplay();
		if(FourthCamera.Translate[2] > 1.0){
			FourthCamera.Translate[2] = 1.0;
			Falpha = 0.001;
			FourthMove[3] = false;
			FourthRota[3] = true;
			glutPostRedisplay();
		}
	}
	else if(FourthRota[3]){
		FourthCamera.Translate[0] += 0.05;
		FourthCamera.Translate[2] -= 0.13;
		FourthCamera.Rotation[1] -= 3.0;
		glutPostRedisplay();
		if(FourthCamera.Rotation[1] < -270.0){
			FourthCamera.Translate[0] = -1.0;
			FourthCamera.Translate[2] = -3.0;
			FourthCamera.Rotation[1] = 90.0;
			Falpha = 0.001;
			FourthRota[3] = false;
			FourthCaptureflag = false;
			glutPostRedisplay();
			MakeVideo(FourthCapture);
		}
	}
}

void Idle(){
	FirstCameraAnimation();
	FourthCameraAnimation();
}

int main(int argc,char** argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGBA|GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(100,100);

	OpenOBJ("Stadium.obj");
	OpenOBJ("metrouloke.obj");

	glutCreateWindow("Open GL : Smart Stadium");
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glutDisplayFunc(SplitDisplay);
	glutReshapeFunc(SplitReshape);
	glutMotionFunc(Motion);
	glutMouseFunc(Mouse);
	glutKeyboardFunc(Keyboard);
	glutIdleFunc(Idle);

	Init();
	glutMainLoop();
}