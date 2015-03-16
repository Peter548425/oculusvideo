// opengl.cpp : Defines the entry point for the console application.
//
//#define NOMINMAX
#include "stdafx.h"
#pragma warning(disable : 4996)
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <opencv2/imgproc/imgproc.hpp>  
#include "opencv2/core/core.hpp"
#include "opencv2/flann/miniflann.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/photo/photo.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/ml/ml.hpp"
#include "opencv2/core/core_c.h"
#include "opencv2/highgui/highgui_c.h"

#include "SDL.h"
#define GLEW_STATIC
#include "GL/glew.h"
#define OVR_OS_WIN32
#include "OVR_CAPI_GL.h"
#include "Kernel/OVR_Math.h"
#include "SDL_syswm.h"
#include "Ovladanie.h"

#include <iostream>
#include <fstream>
#include <thread>
using namespace OVR;
using namespace cv;

GLuint textureCV;
int prvyKrat = 0;
GLuint cvImage(Mat texture_cv);
int load_textures();
GLuint textures[2];
int teplota = 0, rychlost = 0, vlhkost = 0;
char** dataVypis;

Mat recentFrame;
Mat recentFrame2;

void task1(VideoCapture cap1 )						//Nacitava framy z kamery1 a uklada do recentFrame
{
	bool bSuccess;
	while(true){
		try
		{
			bSuccess = cap1.read(recentFrame);
		}catch(int e){}
		
	}
}

void task2(VideoCapture cap2 )						//Nacitava framy z kamery2 a uklada do recentFrame2
{
	bool bSuccess;
	while(true){
		try
		{
			bSuccess = cap2.read(recentFrame2);
		}catch(int e){}
	}
}

void task3Ovaldanie()								//Spusti funkciu ovladanief(), ktora riesi komunikaciu s vozidlom
{
		ovladanief();
}



int main(int argc, char *argv[])
{
        SDL_Init(SDL_INIT_VIDEO);											//Oculus magic
 
        int x = SDL_WINDOWPOS_CENTERED;
        int y = SDL_WINDOWPOS_CENTERED;
        Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
 
        bool debug = false;
 
        ovr_Initialize();
 
        ovrHmd hmd = ovrHmd_Create(0);
 
        if (hmd == NULL)
        {
			hmd = ovrHmd_CreateDebug(ovrHmd_DK1);
 
			debug = true;
        }
 
        if (debug == false)
        {
			x = hmd->WindowsPos.x;
            y = hmd->WindowsPos.y;
            flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
        }
 
        int w = hmd->Resolution.w;
        int h = hmd->Resolution.h;
 
        SDL_Window *window = SDL_CreateWindow("Oculus Rift SDL2 OpenGL Demo", x, y, w, h, flags);
 
        SDL_GLContext context = SDL_GL_CreateContext(window);
 
        glewExperimental = GL_TRUE;
 
        glewInit();
 
        Sizei recommendedTex0Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Left, hmd->DefaultEyeFov[0], 1.0f);
        Sizei recommendedTex1Size = ovrHmd_GetFovTextureSize(hmd, ovrEye_Right, hmd->DefaultEyeFov[1], 1.0f);
        Sizei renderTargetSize;
        renderTargetSize.w = recommendedTex0Size.w + recommendedTex1Size.w;
        renderTargetSize.h = max(recommendedTex0Size.h, recommendedTex1Size.h);
 
        GLuint frameBuffer;
        glGenFramebuffers(1, &frameBuffer);
 
        GLuint texture;
        glGenTextures(1, &texture);
 
        glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, renderTargetSize.w, renderTargetSize.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0);
 
        GLuint renderBuffer;
        glGenRenderbuffers(1, &renderBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, renderTargetSize.w, renderTargetSize.h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
 
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
			glDeleteFramebuffers(1, &frameBuffer);
            glDeleteTextures(1, &texture);
            glDeleteRenderbuffers(1, &renderBuffer);
 
            SDL_GL_DeleteContext(context);
 
            SDL_DestroyWindow(window);
 
            ovrHmd_Destroy(hmd);
 
            ovr_Shutdown();
 
            SDL_Quit();
 
            return 0;
        }
 
        ovrFovPort eyeFov[2] = { hmd->DefaultEyeFov[0], hmd->DefaultEyeFov[1] };
 
        ovrRecti eyeRenderViewport[2];
        eyeRenderViewport[0].Pos = Vector2i(0, 0);
        eyeRenderViewport[0].Size = Sizei(renderTargetSize.w / 2, renderTargetSize.h);
        eyeRenderViewport[1].Pos = Vector2i((renderTargetSize.w + 1) / 2, 0);
        eyeRenderViewport[1].Size = eyeRenderViewport[0].Size;
 
        ovrGLTexture eyeTexture[2];
        eyeTexture[0].OGL.Header.API = ovrRenderAPI_OpenGL;
        eyeTexture[0].OGL.Header.TextureSize = renderTargetSize;
        eyeTexture[0].OGL.Header.RenderViewport = eyeRenderViewport[0];
        eyeTexture[0].OGL.TexId = texture;
 
        eyeTexture[1] = eyeTexture[0];
        eyeTexture[1].OGL.Header.RenderViewport = eyeRenderViewport[1];

		ovrTexture eyeTex[2];

		eyeTex[0] = eyeTexture[0].Texture;
		eyeTex[1] = eyeTexture[1].Texture;
 
        SDL_SysWMinfo info;
 
        SDL_VERSION(&info.version);
 
        SDL_GetWindowWMInfo(window, &info);
 
        ovrGLConfig cfg;
        cfg.OGL.Header.API = ovrRenderAPI_OpenGL;
        cfg.OGL.Header.RTSize = Sizei(hmd->Resolution.w, hmd->Resolution.h);
        cfg.OGL.Header.Multisample = 1;
#if defined(OVR_OS_WIN32)
        if (!(hmd->HmdCaps & ovrHmdCap_ExtendDesktop))
                ovrHmd_AttachToWindow(hmd, info.info.win.window, NULL, NULL);
 
        cfg.OGL.Window = info.info.win.window;
        cfg.OGL.DC = NULL;
#elif defined(OVR_OS_LINUX)
        cfg.OGL.Disp = info.info.x11.display;
        cfg.OGL.Win = info.info.x11.window;
#endif
 
        ovrEyeRenderDesc eyeRenderDesc[2];
 
        ovrHmd_ConfigureRendering(hmd, &cfg.Config, ovrDistortionCap_Chromatic | ovrDistortionCap_Vignette | ovrDistortionCap_TimeWarp | ovrDistortionCap_Overdrive, eyeFov, eyeRenderDesc);
 
        ovrHmd_SetEnabledCaps(hmd, ovrHmdCap_LowPersistence | ovrHmdCap_DynamicPrediction);
 
        ovrHmd_ConfigureTracking(hmd, ovrTrackingCap_Orientation | ovrTrackingCap_MagYawCorrection | ovrTrackingCap_Position, 0);
		
       
		const GLchar *vertexShaderSource[] = {
                "#version 150\n"
                "uniform mat4 MVPMatrix;\n"
				"in vec2 texture_coord;"
				"out vec2 texture_coord_from_vshader;"
                "in vec3 position;\n"
                "void main()\n"
                "{\n"
                "    gl_Position = MVPMatrix * vec4(position, 1.0);\n"
					"texture_coord_from_vshader = texture_coord;"
                "}"
        };
 
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
        glCompileShader(vertexShader);
 
        const GLchar *fragmentShaderSource[] = {
                "#version 150\n"
                "out vec4 outputColor;\n"
				"in vec2 texture_coord_from_vshader;"
				"uniform sampler2D texture_sampler;"
                "void main()\n"
                "{\n"
                "    outputColor = texture(texture_sampler, texture_coord_from_vshader);"
                "}"
        };
 
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
 
        GLuint program = glCreateProgram();
        glAttachShader(program, vertexShader);
        glAttachShader(program, fragmentShader);
        glLinkProgram(program);
        glUseProgram(program);
 
        GLuint MVPMatrixLocation = glGetUniformLocation(program, "MVPMatrix");
        GLuint positionLocation = glGetAttribLocation(program, "position");
 
        GLuint vertexArray;
        glGenVertexArrays(1, &vertexArray);
        glBindVertexArray(vertexArray);
 
        GLfloat vertices[] = {
                -1.0, -1.0, -0.5,
				1.0, -1.0, -0.5,
				1.0, 1.0, -0.5,
				-1.0, 1.0, -0.5,
				
        };

		GLfloat texture_coord[] = {
			0.0, 0.0,
			1.0, 0.0,
			1.0, 1.0,
			0.0, 1.0,
		};

		GLuint indices[6] = {
			0, 1, 2,
			2, 3, 0
		};

		
        GLuint positionBuffer;
        glGenBuffers(1, &positionBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(texture_coord) , vertices, GL_STATIC_DRAW);    // + texture_coord
        // Transfer the vertex positions:
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

		// Transfer the texture coordinates:
		glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(texture_coord), texture_coord);
		glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(positionLocation);
 



		GLuint eab;
		glGenBuffers(1, &eab);
	
		// Transfer the data from indices to eab
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eab);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


		//http://192.168.1.10:8080/video?dummy=video.mjpg
		//VideoCapture cap("D:\\Movies\\Don-Jon-(2013)-ENG.avi"); 
		//VideoCapture cap(0); 
		VideoCapture cap("E:\\Download\\nords.mp4");					//Otvorenie 1. streamu
		if ( !cap.isOpened() )  // if not success, exit program
		{
			std::cout << "Cannot open the video stream1" << std::endl;
         
		}

				
				 
		VideoCapture cap2("E:\\Download\\datweekaz.mp4");					//Otvorenie 2. streamu
		if ( !cap2.isOpened() )  // if not success, exit program
		{
			std::cout << "Cannot open the video stream2" << std::endl;
        }
		
		std::thread t1(task1, cap);				//Start threadov a citanie stale novych framov z kamery
		std::thread t2(task2, cap2);
		std::thread t3(task3Ovaldanie);			//Start threadu, ktory riesi ovladanie vozidla a komunikaciu

		glGenTextures(2, textures);


		// Texture coord attribute
		GLint texture_coord_attribute = glGetAttribLocation(program, "texture_coord");
		glVertexAttribPointer(texture_coord_attribute, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(vertices));
		glEnableVertexAttribArray(texture_coord_attribute);

	

        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClearDepth(1.0f);
 
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_DEPTH_TEST);
 
        bool running = true;
 
        while (running == true)														//Spustenie cyklu
        {
                SDL_Event event;
 
                while (SDL_PollEvent(&event))										//Health sprava
                {
                        switch (event.type)
                        {
                        case SDL_QUIT:
                                running = false;
                                break;
                        case SDL_KEYDOWN:
                                ovrHmd_DismissHSWDisplay(hmd);
 
                                switch (event.key.keysym.sym)
                                {
                                case SDLK_ESCAPE:
                                        running = false;
                                        break;
                                default:
                                        break;
                                }
                                break;
                        default:
                                break;
                        }
                }

				//Prva verzia nacitavania framov
				//Mat frame[2];
				////Mat frame2;
				////cap.retrieve(frame);
				//bool bSuccess = cap.read(frame[0]);
				//bool bSuccess2 = cap2.read(frame[1]);
				//if (bSuccess) //if not success, break loop
				//{
				//	cvImage(frame);
				//}
				//frame[0].release();
				//if (bSuccess2) //if not success, break loop
				//{
				//	textures[1] = cvImage(frame2);
				//}
				//frame[1].release();
				//glBindTexture(GL_TEXTURE_2D, textures[0]);
				//glActiveTexture(GL_TEXTURE1);
				// 
				

				//------------------
                ovrFrameTiming frameTiming = ovrHmd_BeginFrame(hmd, 0);
 
                glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
 
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
 
                ovrPosef eyeRenderPose[2];
 
                for (int eyeIndex = 0; eyeIndex < ovrEye_Count; eyeIndex++)							//Pre kazde oko
                {

					if (eyeIndex == 1)																//Pre oko 1 spusti funkciu cvImage pre frame z kamery1 a opacne
					{
						cvImage(recentFrame2);		//na pravu polku najnovsi frame z kamery2
					}else{
						cvImage(recentFrame);		// na lavu obraz z kamery1
					}

					
                    ovrEyeType eye = hmd->EyeRenderOrder[eyeIndex];
                    eyeRenderPose[eye] = ovrHmd_GetEyePose(hmd, eye);
						
					ovrPosef eyeRenderPoseNula;											//Vytvorenie objektu ovrPosef a naplnenie orientacie, potrebne na to, aby obraz nestal na jednom mieste v priestore, ale bol pre ocami
					eyeRenderPoseNula.Orientation.w = 1;
					eyeRenderPoseNula.Orientation.x = 0;
					eyeRenderPoseNula.Orientation.z = 0;
					eyeRenderPoseNula.Orientation.y = 0;

					//Oculus SDK 0.4.2
                    Matrix4f MVPMatrix = Matrix4f(ovrMatrix4f_Projection(eyeRenderDesc[eye].Fov, 0.01f, 10000.0f, true)) * Matrix4f::Translation(eyeRenderDesc[eye].ViewAdjust) * Matrix4f(Quatf(eyeRenderPoseNula.Orientation).Inverted());
					//Oculus SDK 0.4.3
					//Matrix4f MVPMatrix = Matrix4f(ovrMatrix4f_Projection(eyeRenderDesc[eye].Fov, 0.01f, 10000.0f, true)) * Matrix4f::Translation(eyeRenderDesc[eye].HmdToEyeViewOffset) * Matrix4f(Quatf(eyeRenderPoseNula.Orientation).Inverted());


					
					glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, &MVPMatrix.Transposed().M[0][0]);
 
					
                    glViewport(eyeRenderViewport[eye].Pos.x, eyeRenderViewport[eye].Pos.y, eyeRenderViewport[eye].Size.w, eyeRenderViewport[eye].Size.h);
							
					glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);		//Vykresli texturu
                    
                }
 
                glBindVertexArray(0);
 
                ovrHmd_EndFrame(hmd, eyeRenderPose, eyeTex);
 
                glBindVertexArray(vertexArray);
        }
 
        glDeleteVertexArrays(1, &vertexArray);
        glDeleteBuffers(1, &positionBuffer);
 
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(program);
 
        glDeleteFramebuffers(1, &frameBuffer);
        glDeleteTextures(1, &texture);
        glDeleteRenderbuffers(1, &renderBuffer);
 
        SDL_GL_DeleteContext(context);
 
        SDL_DestroyWindow(window);
 
        ovrHmd_Destroy(hmd);
 
        ovr_Shutdown();
 
        SDL_Quit();
 
        return 0;
}




GLuint cvImage(Mat texture_cv){     //Funkcia na konverziu OpenCV Mat na OpenGL texturu a pridanie textu
	cv::Mat flipped;
	teplota++;
	rychlost++;
	vlhkost++;
	char teplotaa[250] = "";							//Docasne vztvaranie random textu
	sprintf(teplotaa, "Teplota %d", teplota/50);
	char vlkosta[250] = "";
	sprintf(vlkosta, "Vlhkost %d", rychlost/80);
	char rychlosta[5000] = "";


	sprintf(rychlosta, "Data %s",getDataOvladanie());

	cv::putText(texture_cv, teplotaa, cvPoint(460,200), FONT_HERSHEY_COMPLEX_SMALL, 1, cvScalar(200,200,250), 2, CV_AA);	//Vlozenie textu do OpenCV Mat
	cv::putText(texture_cv, vlkosta, cvPoint(460,220), FONT_HERSHEY_COMPLEX_SMALL, 1, cvScalar(200,200,250), 2, CV_AA);		//Vlozenie textu do OpenCV Mat
	cv::putText(texture_cv, rychlosta, cvPoint(460,240), FONT_HERSHEY_COMPLEX_SMALL, 1, cvScalar(200,200,250), 2, CV_AA);	//Vlozenie textu do OpenCV Mat


	cv::flip(texture_cv, flipped, 0);				//Otocenie textury
	texture_cv = flipped;       




	if(prvyKrat == 0){
		glGenTextures(1, &textureCV);                  //Ak sa funkcia spustila 1. krat, tak vztvor OpenGL texturu, treba spustit len raz, inac sa zahlti RAM
		prvyKrat++;
	}

		
    glBindTexture(GL_TEXTURE_2D, textureCV);            //Namapovanie Mat do OpenGL textury
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); 
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); 
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S , GL_REPEAT );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, texture_cv.cols, texture_cv.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, texture_cv.data);
		
	
	texture_cv.release();    
	flipped.release();
	return textureCV;
  
}

