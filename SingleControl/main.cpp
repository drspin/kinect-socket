/****************************************************************************
*                                                                           *
*   Nite 1.3 - Single Control Sample                                        *
*                                                                           *
*   Author:     Oz Magal                                                    *
*                                                                           *
****************************************************************************/

/****************************************************************************
*									    *
*  Single Control with unix socket client				    *
*									    *
*  Unix socket added by: Kevin Ewe					    *
*									    *
*  Inspired by: BlitzLabs	   			                    *
*								 	    *
****************************************************************************/

/****************************************************************************
*                                                                           *
*   Nite 1.3	                                                            *
*   Copyright (C) 2006 PrimeSense Ltd. All Rights Reserved.                 *
*                                                                           *
*   This file has been provided pursuant to a License Agreement containing  *
*   restrictions on its use. This data contains valuable trade secrets      *
*   and proprietary information of PrimeSense Ltd. and is protected by law. *
*                                                                           *
****************************************************************************/

//-----------------------------------------------------------------------------
// Headers
//-----------------------------------------------------------------------------
// General headers
#include <stdio.h>
// OpenNI headers
#include <XnOpenNI.h>
// NITE headers
#include <XnVSessionManager.h>
#include "XnVMultiProcessFlowClient.h"
#include <XnVWaveDetector.h>

#include "kbhit.h"
#include "signal_catch.h"

// Headers for unix socket
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

// xml to initialize OpenNI
#define SAMPLE_XML_FILE "../../Data/Sample-Tracking.xml"

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT 8124

XnBool g_bQuit = false;

int sockfd;

//-----------------------------------------------------------------------------
// Callbacks
//-----------------------------------------------------------------------------

// Callback for when the focus is in progress
void XN_CALLBACK_TYPE SessionProgress(const XnChar* strFocus, const XnPoint3D& ptFocusPoint, XnFloat fProgress, void* UserCxt)
{
	printf("Session progress (%6.2f,%6.2f,%6.2f) - %6.2f [%s]\n", ptFocusPoint.X, ptFocusPoint.Y, ptFocusPoint.Z, fProgress,  strFocus);
}
// callback for session start
void XN_CALLBACK_TYPE SessionStart(const XnPoint3D& ptFocusPoint, void* UserCxt)
{
	printf("Session started. Please wave (%6.2f,%6.2f,%6.2f)...\n", ptFocusPoint.X, ptFocusPoint.Y, ptFocusPoint.Z);
}
// Callback for session end
void XN_CALLBACK_TYPE SessionEnd(void* UserCxt)
{
	printf("Session ended. Please perform focus gesture to start session\n");
}
// Callback for wave detection
void XN_CALLBACK_TYPE OnWaveCB(void* cxt)
{
	printf("Wave!\n");
}

// callback for a new position of any hand
void XN_CALLBACK_TYPE OnPointUpdate(const XnVHandPointContext* pContext, void* cxt)
{
	//------------------------------------------------------------------------------- //
	// ------------------------- SEND DATA TO SOCKET SERVER ------------------------  //
	//------------------------------------------------------------------------------- //
	
	// Allocate memory for the string we will send to the socket server.
	// length will be the size of the mem allocation for the string
	int length = snprintf(NULL,0 , "%d,%f,%f,%f,%f", pContext->nID, pContext->ptPosition.X, pContext->ptPosition.Y, pContext->ptPosition.Z, pContext->fTime);
	
	// Character object that will store the string
	char * data = (char*) malloc((length + 1) * sizeof(char));
	
	// Print string in format of: [id,x,y,x,time]
	snprintf(data, length, "%d,%f,%f,%f,%f", pContext->nID, pContext->ptPosition.X, pContext->ptPosition.Y, pContext->ptPosition.Z, pContext->fTime);
	
	// Send data off to socket server
	send( sockfd, data, (length + 1), 0 );

	// Free up the memory used for the string
	free(data);

	// Print the same data to console.
	printf("%d: (%f,%f,%f) [%f]\n", pContext->nID, pContext->ptPosition.X, pContext->ptPosition.Y, pContext->ptPosition.Z, pContext->fTime);
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

// this sample can run either as a regular sample, or as a client for multi-process (remote mode)
int main(int argc, char** argv)
{

	//------------------------------------------------------------------------------- //
	// ---------------------- SOCKET SERVER INITIALIZATION -------------------------  //
	//------------------------------------------------------------------------------- //
	
	// This application will fail if your socket server is not running when starting the app
	//--------------------------------------------------------------------------------------
	
	printf("attempting to connect to socket server...\n");

	int n;
	struct sockaddr_in serv_addr;
	char hello[] = "knock knock";

  /* First call to socket() function */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) 
  {
      perror("ERROR opening socket");
      exit(1);
  }

  /* Initialize socket structure */
  bzero((char *) &serv_addr, sizeof(serv_addr));
  // serv_addr.sin_family = AF_INET;
	serv_addr.sin_family = AF_UNSPEC;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(DEFAULT_PORT);
  
	// connecting to socket
	 if (connect(sockfd, (struct sockaddr *) &serv_addr,
	                        sizeof(serv_addr)) < 0)
	  {
	       perror("ERROR on connection");
	       exit(1);
	  }
	printf("Connected\n");
	
  /* Write a response to the client */
  // n = write(sockfd,"Knock knock",18);
	n = send(sockfd, hello, (int)strlen(hello), 0);
	  if (n < 0)
	  {
	      perror("ERROR writing to socket");
	      exit(1);
	  } 
	// close(sockfd);
  // return 0;
	
	//------------------------------------------------------------------------------- //
	// ---------------------- END SOCKET SERVER CODE -------------------------------  //
	//------------------------------------------------------------------------------- //


	xn::Context context;
	XnVSessionGenerator* pSessionGenerator;
	XnBool bRemoting = FALSE;

	if (argc > 1)
	{
		// remote mode
		context.Init();
		printf("Running in 'Remoting' mode (Section name: %s)\n", argv[1]);
		bRemoting = TRUE;

		// Create multi-process client
		pSessionGenerator = new XnVMultiProcessFlowClient(argv[1]);

		XnStatus rc = ((XnVMultiProcessFlowClient*)pSessionGenerator)->Initialize();
		if (rc != XN_STATUS_OK)
		{
			printf("Initialize failed: %s\n", xnGetStatusString(rc));
			delete pSessionGenerator;
			return 1;
		}
	}
	else
	{
		// Local mode
		// Create context
		XnStatus rc = context.InitFromXmlFile(SAMPLE_XML_FILE);
		if (rc != XN_STATUS_OK)
		{
			printf("Couldn't initialize: %s\n", xnGetStatusString(rc));
			return 1;
		}

		// Create the Session Manager
		pSessionGenerator = new XnVSessionManager();
		rc = ((XnVSessionManager*)pSessionGenerator)->Initialize(&context, "Click", "RaiseHand");
		if (rc != XN_STATUS_OK)
		{
			printf("Session Manager couldn't initialize: %s\n", xnGetStatusString(rc));
			delete pSessionGenerator;
			return 1;
		}

		// Initialization done. Start generating
		context.StartGeneratingAll();
	}

	// Register session callbacks
	pSessionGenerator->RegisterSession(NULL, &SessionStart, &SessionEnd, &SessionProgress);

	// Start catching signals for quit indications
	CatchSignals(&g_bQuit);

	// init & register wave control
	XnVWaveDetector wc;
	wc.RegisterWave(NULL, OnWaveCB);
	wc.RegisterPointUpdate(NULL, OnPointUpdate);
	pSessionGenerator->AddListener(&wc);

	printf("Please perform focus gesture to start session\n");
	printf("Hit any key to exit\n");

	// Main loop
	while ((!_kbhit()) && (!g_bQuit))
	{
		if (bRemoting)
		{
			((XnVMultiProcessFlowClient*)pSessionGenerator)->ReadState();
		}
		else
		{
			context.WaitAndUpdateAll();
			((XnVSessionManager*)pSessionGenerator)->Update(&context);
		}
	}

	delete pSessionGenerator;

	context.Shutdown();

	return 0;
}
