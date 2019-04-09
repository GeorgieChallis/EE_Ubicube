///////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) OMG Plc 2009.
// All rights reserved.  This software is protected by copyright
// law and international treaties.  No part of this software / document
// may be reproduced or distributed in any form or by any means,
// whether transiently or incidentally to some other use of this software,
// without the written permission of the copyright owner.
//
///////////////////////////////////////////////////////////////////////////////
#define _CRT_SECURE_NO_WARNINGS
#include "DataStreamClient.h"

#include <stdio.h>

#include <iostream>
#include <fstream>
#include <cassert>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <string.h>

#ifdef WIN32
  #include <conio.h>   // For _kbhit()
  #include <cstdio>   // For getchar()
  #include <windows.h> // For Sleep()
#else
#include <conio.h>   // For _kbhit()
#include <cstdio>   // For getchar()
#include <windows.h> // For Sleep()
 // #include <unistd.h> // For sleep()
#endif // WIN32

#include <time.h>

using namespace ViconDataStreamSDK::CPP;

#define output_stream if(!LogFile.empty()) ; else std::cout 

namespace
{
  std::string Adapt( const bool i_Value )
  {
    return i_Value ? "True" : "False";
  }

  std::string Adapt( const TimecodeStandard::Enum i_Standard )
  {
    switch( i_Standard )
    {
      default:
    case TimecodeStandard::None:
        return "0";
    case TimecodeStandard::PAL:
        return "1";
    case TimecodeStandard::NTSC:
        return "2";
    case TimecodeStandard::NTSCDrop:
        return "3";
    case TimecodeStandard::Film:
        return "4";
    case TimecodeStandard::NTSCFilm:
        return "5";
    case TimecodeStandard::ATSC:
        return "6";
    }
  }

  std::string Adapt( const Direction::Enum i_Direction )
  {
    switch( i_Direction )
    {
      case Direction::Forward:
        return "Forward";
      case Direction::Backward:
        return "Backward";
      case Direction::Left:
        return "Left";
      case Direction::Right:
        return "Right";
      case Direction::Up:
        return "Up";
      case Direction::Down:
        return "Down";
      default:
        return "Unknown";
    }
  }

  std::string Adapt( const DeviceType::Enum i_DeviceType )
  {
    switch( i_DeviceType )
    {
      case DeviceType::ForcePlate:
        return "ForcePlate";
      case DeviceType::Unknown:
      default:
        return "Unknown";
    }
  }

  std::string Adapt( const Unit::Enum i_Unit )
  {
    switch( i_Unit )
    {
      case Unit::Meter:
        return "Meter";
      case Unit::Volt:
        return "Volt";
      case Unit::NewtonMeter:
        return "NewtonMeter";
      case Unit::Newton:
        return "Newton";
      case Unit::Kilogram:
        return "Kilogram";
      case Unit::Second:
        return "Second";
      case Unit::Ampere:
        return "Ampere";
      case Unit::Kelvin:
        return "Kelvin";
      case Unit::Mole:
        return "Mole";
      case Unit::Candela:
        return "Candela";
      case Unit::Radian:
        return "Radian";
      case Unit::Steradian:
        return "Steradian";
      case Unit::MeterSquared:
        return "MeterSquared";
      case Unit::MeterCubed:
        return "MeterCubed";
      case Unit::MeterPerSecond:
        return "MeterPerSecond";
      case Unit::MeterPerSecondSquared:
        return "MeterPerSecondSquared";
      case Unit::RadianPerSecond:
        return "RadianPerSecond";
      case Unit::RadianPerSecondSquared:
        return "RadianPerSecondSquared";
      case Unit::Hertz:
        return "Hertz";
      case Unit::Joule:
        return "Joule";
      case Unit::Watt:
        return "Watt";
      case Unit::Pascal:
        return "Pascal";
      case Unit::Lumen:
        return "Lumen";
      case Unit::Lux:
        return "Lux";
      case Unit::Coulomb:
        return "Coulomb";
      case Unit::Ohm:
        return "Ohm";
      case Unit::Farad:
        return "Farad";
      case Unit::Weber:
        return "Weber";
      case Unit::Tesla:
        return "Tesla";
      case Unit::Henry:
        return "Henry";
      case Unit::Siemens:
        return "Siemens";
      case Unit::Becquerel:
        return "Becquerel";
      case Unit::Gray:
        return "Gray";
      case Unit::Sievert:
        return "Sievert";
      case Unit::Katal:
        return "Katal";

      case Unit::Unknown:
      default:
        return "Unknown";
    }
  }
#ifdef WIN32
  bool Hit()
  {
    bool hit = false;
    while( _kbhit() )
    {
      getchar();
      hit = true;
    }
    return hit;
  }
#endif
}

String markerPos = "markerposition.txt";



int main( int argc, char* argv[] )
{
  // Program options
  
	std::string HostName = "localhost:801"; //"134.225.86.151"
  if( argc > 1 )
  {
    HostName = argv[1];
  }

  // log contains:
  // version number
  // log of framerate over time
  // --multicast
  // kill off internal app
  std::string LogFile = "";

  unsigned int ClientBufferSize = 0;
  std::string AxisMapping = "ZUp";


  std::ofstream ofs;
  if(!LogFile.empty())
  {
    ofs.open(LogFile.c_str());
    if(!ofs.is_open())
    {
      std::cout << "Could not open log file <" << LogFile << ">...exiting" << std::endl;
      return 1;
    }
  }
  // Make a new client
  ViconDataStreamSDK::CPP::Client MyClient;

  for(int i=0; i != 3; ++i) // repeat to check disconnecting doesn't wreck next connect
  {
    // Connect to a server
    std::cout << "Connecting to " << HostName << " ..." << std::flush;
    while( !MyClient.IsConnected().Connected )
    {
      // Direct connection

      bool ok = false;

      ok =( MyClient.Connect( HostName ).Result == Result::Success );
      
      if(!ok)
      {
        std::cout << "Warning - connect failed..." << std::endl;
      }
      std::cout << ".";
  #ifdef WIN32
      Sleep( 1000 );
  #else
	  Sleep(200);
	   //sleep(1);
  #endif
    }
    std::cout << std::endl;

    // Enable some different data types
    MyClient.EnableSegmentData();
    MyClient.EnableMarkerData();
    MyClient.EnableUnlabeledMarkerData();
   // MyClient.EnableMarkerRayData();
   // MyClient.EnableDeviceData();
   // MyClient.EnableDebugData();
   

    std::cout << "Segment Data Enabled: "          << Adapt( MyClient.IsSegmentDataEnabled().Enabled )         << std::endl;
    std::cout << "Marker Data Enabled: "           << Adapt( MyClient.IsMarkerDataEnabled().Enabled )          << std::endl;
    std::cout << "Unlabeled Marker Data Enabled: " << Adapt( MyClient.IsUnlabeledMarkerDataEnabled().Enabled ) << std::endl;
    //std::cout << "Device Data Enabled: "           << Adapt( MyClient.IsDeviceDataEnabled().Enabled )          << std::endl;
    //std::cout << "Centroid Data Enabled: "         << Adapt( MyClient.IsCentroidDataEnabled().Enabled )        << std::endl;
    //std::cout << "Marker Ray Data Enabled: "       << Adapt( MyClient.IsMarkerRayDataEnabled().Enabled )       << std::endl;
    //std::cout << "Centroid Data Enabled: "         << Adapt( MyClient.IsCentroidDataEnabled().Enabled)         << std::endl;
   //std::cout << "Greyscale Data Enabled: "        << Adapt( MyClient.IsGreyscaleDataEnabled().Enabled)        << std::endl;
    //std::cout << "Video Data Enabled: "            << Adapt( MyClient.IsVideoDataEnabled().Enabled)            << std::endl;
    //std::cout << "Debug Data Enabled: "            << Adapt( MyClient.IsDebugDataEnabled().Enabled)            << std::endl;

    // Set the streaming mode
    //MyClient.SetStreamMode( ViconDataStreamSDK::CPP::StreamMode::ClientPull );
    // MyClient.SetStreamMode( ViconDataStreamSDK::CPP::StreamMode::ClientPullPreFetch );
    MyClient.SetStreamMode( ViconDataStreamSDK::CPP::StreamMode::ServerPush );

    // Set the global up axis
    MyClient.SetAxisMapping( Direction::Forward, 
                             Direction::Left, 
                             Direction::Up ); // Z-up

    if( AxisMapping == "YUp")
    {
      MyClient.SetAxisMapping( Direction::Forward, 
                               Direction::Up, 
                               Direction::Right ); // Y-up
    }
    else if( AxisMapping == "XUp")
    {
      MyClient.SetAxisMapping( Direction::Up, 
                               Direction::Forward, 
                               Direction::Left ); // Y-up
    }

    Output_GetAxisMapping _Output_GetAxisMapping = MyClient.GetAxisMapping();
    std::cout << "Axis Mapping: X-" << Adapt( _Output_GetAxisMapping.XAxis ) 
                           << " Y-" << Adapt( _Output_GetAxisMapping.YAxis ) 
                           << " Z-" << Adapt( _Output_GetAxisMapping.ZAxis ) << std::endl;

    // Discover the version number
    Output_GetVersion _Output_GetVersion = MyClient.GetVersion();
    std::cout << "Version: " << _Output_GetVersion.Major << "." 
                             << _Output_GetVersion.Minor << "." 
                             << _Output_GetVersion.Point << std::endl;

    if( ClientBufferSize > 0 )
    {
      MyClient.SetBufferSize( ClientBufferSize );
      std::cout << "Setting client buffer size to " << ClientBufferSize << std::endl;
    }

    size_t FrameRateWindow = 1000; // frames
    size_t Counter = 0;
    clock_t LastTime = clock();
    // Loop until a key is pressed
  #ifdef WIN32
    while( !Hit() )
  #else
    while( true)
  #endif
    {
      // Get a frame
      output_stream << "Waiting for new frame...";
      while( MyClient.GetFrame().Result != Result::Success )
      {
        // Sleep a little so that we don't lumber the CPU with a busy poll
        #ifdef WIN32
          Sleep( 200 );
        #else
		  Sleep(200);
		   //sleep(1);
        #endif

        output_stream << ".";
      }
      output_stream << std::endl;
	  if (++Counter == FrameRateWindow)
	  {
		  clock_t Now = clock();
		  double FrameRate = (double)(FrameRateWindow * CLOCKS_PER_SEC) / (double)(Now - LastTime);
		  if (!LogFile.empty())
		  {
			  time_t rawtime;
			  struct tm * timeinfo;
			  time(&rawtime);
			  timeinfo = localtime(&rawtime);

			  ofs << "Frame rate = " << FrameRate << " at " << asctime(timeinfo) << std::endl;
		  }

		  LastTime = Now;
		  Counter = 0;
		  }

	  // Get the frame number
	  Output_GetFrameNumber _Output_GetFrameNumber = MyClient.GetFrameNumber();
	  output_stream << "Frame Number: " << _Output_GetFrameNumber.FrameNumber << std::endl;

	  Output_GetFrameRate Rate = MyClient.GetFrameRate();
	  std::cout << "Frame rate: " << Rate.FrameRateHz << std::endl;

	  // Show frame rates
	  for (unsigned int FramerateIndex = 0; FramerateIndex < MyClient.GetFrameRateCount().Count; ++FramerateIndex)
	  {
		  std::string FramerateName = MyClient.GetFrameRateName(FramerateIndex).Name;
		  double      FramerateValue = MyClient.GetFrameRateValue(FramerateName).Value;

		  output_stream << FramerateName << ": " << FramerateValue << "Hz" << std::endl;
	  }
	  output_stream << std::endl;

	  // Get the timecode
	  Output_GetTimecode _Output_GetTimecode = MyClient.GetTimecode();

	  output_stream << "Timecode: "
		  << _Output_GetTimecode.Hours << "h "
		  << _Output_GetTimecode.Minutes << "m "
		  << _Output_GetTimecode.Seconds << "s "
		  << _Output_GetTimecode.Frames << "f "
		  << _Output_GetTimecode.SubFrame << "sf "
		  << Adapt(_Output_GetTimecode.FieldFlag) << " "
		  << Adapt(_Output_GetTimecode.Standard) << " "
		  << _Output_GetTimecode.SubFramesPerFrame << " "
		  << _Output_GetTimecode.UserBits << std::endl << std::endl;

	  // Get the latency
	  output_stream << "Latency: " << MyClient.GetLatencyTotal().Total << "s" << std::endl;

	  for (unsigned int LatencySampleIndex = 0; LatencySampleIndex < MyClient.GetLatencySampleCount().Count; ++LatencySampleIndex)
	  {
		  std::string SampleName = MyClient.GetLatencySampleName(LatencySampleIndex).Name;
		  double      SampleValue = MyClient.GetLatencySampleValue(SampleName).Value;

		  output_stream << "  " << SampleName << " " << SampleValue << "s" << std::endl;
	  }
	  output_stream << std::endl;

	  Output_GetHardwareFrameNumber _Output_GetHardwareFrameNumber = MyClient.GetHardwareFrameNumber();
	  output_stream << "Hardware Frame Number: " << _Output_GetHardwareFrameNumber.HardwareFrameNumber << std::endl;

	  // Count the number of subjects
	  unsigned int SubjectCount = MyClient.GetSubjectCount().SubjectCount;
	  output_stream << "Subjects (" << SubjectCount << "):" << std::endl;
	  for (unsigned int SubjectIndex = 0; SubjectIndex < SubjectCount; ++SubjectIndex)
	  {
		  output_stream << "  Subject #" << SubjectIndex << std::endl;

		  // Get the subject name
		  std::string SubjectName = MyClient.GetSubjectName(SubjectIndex).SubjectName;
		  output_stream << "    Name: " << SubjectName << std::endl;

		  // Get the root segment
		  std::string RootSegment = MyClient.GetSubjectRootSegmentName(SubjectName).SegmentName;
		  output_stream << "    Root Segment: " << RootSegment << std::endl;

		  // Count the number of segments
		  unsigned int SegmentCount = MyClient.GetSegmentCount(SubjectName).SegmentCount;
		  output_stream << "    Segments (" << SegmentCount << "):" << std::endl;
		  for (unsigned int SegmentIndex = 0; SegmentIndex < SegmentCount; ++SegmentIndex)
		  {
			  output_stream << "      Segment #" << SegmentIndex << std::endl;

			  // Get the segment name
			  std::string SegmentName = MyClient.GetSegmentName(SubjectName, SegmentIndex).SegmentName;
			  output_stream << "        Name: " << SegmentName << std::endl;

			  // Get the segment parent
			  std::string SegmentParentName = MyClient.GetSegmentParentName(SubjectName, SegmentName).SegmentName;
			  output_stream << "        Parent: " << SegmentParentName << std::endl;

			  // Get the segment's children
			  unsigned int ChildCount = MyClient.GetSegmentChildCount(SubjectName, SegmentName).SegmentCount;
			  output_stream << "     Children (" << ChildCount << "):" << std::endl;
			  for (unsigned int ChildIndex = 0; ChildIndex < ChildCount; ++ChildIndex)
			  {
				  std::string ChildName = MyClient.GetSegmentChildName(SubjectName, SegmentName, ChildIndex).SegmentName;
				  output_stream << "       " << ChildName << std::endl;
			  }

			  // Get the static segment rotation as a matrix
			  Output_GetSegmentStaticRotationMatrix _Output_GetSegmentStaticRotationMatrix =
				  MyClient.GetSegmentStaticRotationMatrix(SubjectName, SegmentName);
			  output_stream << "        Static Rotation Matrix: (" << _Output_GetSegmentStaticRotationMatrix.Rotation[0] << ", "
				  << _Output_GetSegmentStaticRotationMatrix.Rotation[1] << ", "
				  << _Output_GetSegmentStaticRotationMatrix.Rotation[2] << ", "
				  << _Output_GetSegmentStaticRotationMatrix.Rotation[3] << ", "
				  << _Output_GetSegmentStaticRotationMatrix.Rotation[4] << ", "
				  << _Output_GetSegmentStaticRotationMatrix.Rotation[5] << ", "
				  << _Output_GetSegmentStaticRotationMatrix.Rotation[6] << ", "
				  << _Output_GetSegmentStaticRotationMatrix.Rotation[7] << ", "
				  << _Output_GetSegmentStaticRotationMatrix.Rotation[8] << ")" << std::endl;

			  // Get the static segment rotation in quaternion co-ordinates
			  Output_GetSegmentStaticRotationQuaternion _Output_GetSegmentStaticRotationQuaternion =
				  MyClient.GetSegmentStaticRotationQuaternion(SubjectName, SegmentName);
			  output_stream << "        Static Rotation Quaternion: (" << _Output_GetSegmentStaticRotationQuaternion.Rotation[0] << ", "
				  << _Output_GetSegmentStaticRotationQuaternion.Rotation[1] << ", "
				  << _Output_GetSegmentStaticRotationQuaternion.Rotation[2] << ", "
				  << _Output_GetSegmentStaticRotationQuaternion.Rotation[3] << ")" << std::endl;

			  // Get the static segment rotation in EulerXYZ co-ordinates
			 // Output_GetSegmentStaticRotationEulerXYZ _Output_GetSegmentStaticRotationEulerXYZ = 
			  //  MyClient.GetSegmentStaticRotationEulerXYZ( SubjectName, SegmentName );
			  //output_stream << "        Static Rotation EulerXYZ: (" << _Output_GetSegmentStaticRotationEulerXYZ.Rotation[ 0 ]     << ", " 
														   //      << _Output_GetSegmentStaticRotationEulerXYZ.Rotation[ 1 ]     << ", " 
															//     << _Output_GetSegmentStaticRotationEulerXYZ.Rotation[ 2 ]     << ")" << std::endl;

			  // Get the global segment translation
			  Output_GetSegmentGlobalTranslation _Output_GetSegmentGlobalTranslation =
				  MyClient.GetSegmentGlobalTranslation(SubjectName, SegmentName);
			  output_stream << "        Global Translation: (" << _Output_GetSegmentGlobalTranslation.Translation[0] << ", "
				  << _Output_GetSegmentGlobalTranslation.Translation[1] << ", "
				  << _Output_GetSegmentGlobalTranslation.Translation[2] << ") "
				  << Adapt(_Output_GetSegmentGlobalTranslation.Occluded) << std::endl;

			  // Get the global segment rotation in helical co-ordinates
			  Output_GetSegmentGlobalRotationHelical _Output_GetSegmentGlobalRotationHelical =
				  MyClient.GetSegmentGlobalRotationHelical(SubjectName, SegmentName);
			  output_stream << "        Global Rotation Helical: (" << _Output_GetSegmentGlobalRotationHelical.Rotation[0] << ", "
				  << _Output_GetSegmentGlobalRotationHelical.Rotation[1] << ", "
				  << _Output_GetSegmentGlobalRotationHelical.Rotation[2] << ") "
				  << Adapt(_Output_GetSegmentGlobalRotationHelical.Occluded) << std::endl;

			  // Get the global segment rotation as a matrix
			  Output_GetSegmentGlobalRotationMatrix _Output_GetSegmentGlobalRotationMatrix =
				  MyClient.GetSegmentGlobalRotationMatrix(SubjectName, SegmentName);
			  output_stream << "        Global Rotation Matrix: (" << _Output_GetSegmentGlobalRotationMatrix.Rotation[0] << ", "
				  << _Output_GetSegmentGlobalRotationMatrix.Rotation[1] << ", "
				  << _Output_GetSegmentGlobalRotationMatrix.Rotation[2] << ", "
				  << _Output_GetSegmentGlobalRotationMatrix.Rotation[3] << ", "
				  << _Output_GetSegmentGlobalRotationMatrix.Rotation[4] << ", "
				  << _Output_GetSegmentGlobalRotationMatrix.Rotation[5] << ", "
				  << _Output_GetSegmentGlobalRotationMatrix.Rotation[6] << ", "
				  << _Output_GetSegmentGlobalRotationMatrix.Rotation[7] << ", "
				  << _Output_GetSegmentGlobalRotationMatrix.Rotation[8] << ") "
				  << Adapt(_Output_GetSegmentGlobalRotationMatrix.Occluded) << std::endl;

			  // Get the global segment rotation in quaternion co-ordinates
			  Output_GetSegmentGlobalRotationQuaternion _Output_GetSegmentGlobalRotationQuaternion =
				  MyClient.GetSegmentGlobalRotationQuaternion(SubjectName, SegmentName);
			  output_stream << "        Global Rotation Quaternion: (" << _Output_GetSegmentGlobalRotationQuaternion.Rotation[0] << ", "
				  << _Output_GetSegmentGlobalRotationQuaternion.Rotation[1] << ", "
				  << _Output_GetSegmentGlobalRotationQuaternion.Rotation[2] << ", "
				  << _Output_GetSegmentGlobalRotationQuaternion.Rotation[3] << ") "
				  << Adapt(_Output_GetSegmentGlobalRotationQuaternion.Occluded) << std::endl;

			  // Get the global segment rotation in EulerXYZ co-ordinates
			  Output_GetSegmentGlobalRotationEulerXYZ _Output_GetSegmentGlobalRotationEulerXYZ =
				  MyClient.GetSegmentGlobalRotationEulerXYZ(SubjectName, SegmentName);
			  output_stream << "        Global Rotation EulerXYZ: (" << _Output_GetSegmentGlobalRotationEulerXYZ.Rotation[0] << ", "
				  << _Output_GetSegmentGlobalRotationEulerXYZ.Rotation[1] << ", "
				  << _Output_GetSegmentGlobalRotationEulerXYZ.Rotation[2] << ") "
				  << Adapt(_Output_GetSegmentGlobalRotationEulerXYZ.Occluded) << std::endl;

			  // Get the local segment translation
			  Output_GetSegmentLocalTranslation _Output_GetSegmentLocalTranslation =
				  MyClient.GetSegmentLocalTranslation(SubjectName, SegmentName);
			  output_stream << "        Local Translation: (" << _Output_GetSegmentLocalTranslation.Translation[0] << ", "
				  << _Output_GetSegmentLocalTranslation.Translation[1] << ", "
				  << _Output_GetSegmentLocalTranslation.Translation[2] << ") "
				  << Adapt(_Output_GetSegmentLocalTranslation.Occluded) << std::endl;

			  // Get the local segment rotation in helical co-ordinates
			  Output_GetSegmentLocalRotationHelical _Output_GetSegmentLocalRotationHelical =
				  MyClient.GetSegmentLocalRotationHelical(SubjectName, SegmentName);
			  output_stream << "        Local Rotation Helical: (" << _Output_GetSegmentLocalRotationHelical.Rotation[0] << ", "
				  << _Output_GetSegmentLocalRotationHelical.Rotation[1] << ", "
				  << _Output_GetSegmentLocalRotationHelical.Rotation[2] << ") "
				  << Adapt(_Output_GetSegmentLocalRotationHelical.Occluded) << std::endl;

			  // Get the local segment rotation as a matrix
			  Output_GetSegmentLocalRotationMatrix _Output_GetSegmentLocalRotationMatrix =
				  MyClient.GetSegmentLocalRotationMatrix(SubjectName, SegmentName);
			  output_stream << "        Local Rotation Matrix: (" << _Output_GetSegmentLocalRotationMatrix.Rotation[0] << ", "
				  << _Output_GetSegmentLocalRotationMatrix.Rotation[1] << ", "
				  << _Output_GetSegmentLocalRotationMatrix.Rotation[2] << ", "
				  << _Output_GetSegmentLocalRotationMatrix.Rotation[3] << ", "
				  << _Output_GetSegmentLocalRotationMatrix.Rotation[4] << ", "
				  << _Output_GetSegmentLocalRotationMatrix.Rotation[5] << ", "
				  << _Output_GetSegmentLocalRotationMatrix.Rotation[6] << ", "
				  << _Output_GetSegmentLocalRotationMatrix.Rotation[7] << ", "
				  << _Output_GetSegmentLocalRotationMatrix.Rotation[8] << ") "
				  << Adapt(_Output_GetSegmentLocalRotationMatrix.Occluded) << std::endl;

			  // Get the local segment rotation in quaternion co-ordinates
			  Output_GetSegmentLocalRotationQuaternion _Output_GetSegmentLocalRotationQuaternion =
				  MyClient.GetSegmentLocalRotationQuaternion(SubjectName, SegmentName);
			  output_stream << "        Local Rotation Quaternion: (" << _Output_GetSegmentLocalRotationQuaternion.Rotation[0] << ", "
				  << _Output_GetSegmentLocalRotationQuaternion.Rotation[1] << ", "
				  << _Output_GetSegmentLocalRotationQuaternion.Rotation[2] << ", "
				  << _Output_GetSegmentLocalRotationQuaternion.Rotation[3] << ") "
				  << Adapt(_Output_GetSegmentLocalRotationQuaternion.Occluded) << std::endl;

			  // Get the local segment rotation in EulerXYZ co-ordinates
			  Output_GetSegmentLocalRotationEulerXYZ _Output_GetSegmentLocalRotationEulerXYZ =
				  MyClient.GetSegmentLocalRotationEulerXYZ(SubjectName, SegmentName);
			  output_stream << "        Local Rotation EulerXYZ: (" << _Output_GetSegmentLocalRotationEulerXYZ.Rotation[0] << ", "
				  << _Output_GetSegmentLocalRotationEulerXYZ.Rotation[1] << ", "
				  << _Output_GetSegmentLocalRotationEulerXYZ.Rotation[2] << ") "
				  << Adapt(_Output_GetSegmentLocalRotationEulerXYZ.Occluded) << std::endl;
		  }

		  // Get the quality of the subject (object) if supported
		  Output_GetObjectQuality _Output_GetObjectQuality = MyClient.GetObjectQuality(SubjectName);
		  if (_Output_GetObjectQuality.Result == Result::Success)
		  {
			  double Quality = _Output_GetObjectQuality.Quality;
			  output_stream << "    Quality: " << Quality << std::endl;
		  }

		  // Count the number of markers
		  unsigned int MarkerCount = MyClient.GetMarkerCount(SubjectName).MarkerCount;
		  output_stream << "    Markers (" << MarkerCount << "):" << std::endl;
		  for (unsigned int MarkerIndex = 0; MarkerIndex < MarkerCount; ++MarkerIndex)
		  {
			  // Get the marker name
			  std::string MarkerName = MyClient.GetMarkerName(SubjectName, MarkerIndex).MarkerName;

			  // Get the marker parent
			  std::string MarkerParentName = MyClient.GetMarkerParentName(SubjectName, MarkerName).SegmentName;

			  // Get the global marker translation
			  Output_GetMarkerGlobalTranslation _Output_GetMarkerGlobalTranslation =
				  MyClient.GetMarkerGlobalTranslation(SubjectName, MarkerName);

			  output_stream << "      Marker #" << MarkerIndex << ": "
				  << MarkerName << " ("
				  << _Output_GetMarkerGlobalTranslation.Translation[0] << ", "
				  << _Output_GetMarkerGlobalTranslation.Translation[1] << ", "
				  << _Output_GetMarkerGlobalTranslation.Translation[2] << ") "
				  << Adapt(_Output_GetMarkerGlobalTranslation.Occluded) << std::endl;
		  }
	  }

	  // Get the unlabeled markers
	  unsigned int UnlabeledMarkerCount = MyClient.GetUnlabeledMarkerCount().MarkerCount;
	  output_stream << "    Unlabeled Markers (" << UnlabeledMarkerCount << "):" << std::endl;
	  for (unsigned int UnlabeledMarkerIndex = 0; UnlabeledMarkerIndex < UnlabeledMarkerCount; ++UnlabeledMarkerIndex)
	  {
		  // Get the global marker translation
		  Output_GetUnlabeledMarkerGlobalTranslation _Output_GetUnlabeledMarkerGlobalTranslation =
			  MyClient.GetUnlabeledMarkerGlobalTranslation(UnlabeledMarkerIndex);

		  output_stream << "      Marker #" << UnlabeledMarkerIndex << ": ("
			  << _Output_GetUnlabeledMarkerGlobalTranslation.Translation[0] << ", "
			  << _Output_GetUnlabeledMarkerGlobalTranslation.Translation[1] << ", "
			  << _Output_GetUnlabeledMarkerGlobalTranslation.Translation[2] << ")" << std::endl;
	  }

	  // Get the labeled markers
	  unsigned int LabeledMarkerCount = MyClient.GetLabeledMarkerCount().MarkerCount;
	  output_stream << "    Labeled Markers (" << LabeledMarkerCount << "):" << std::endl;
	  for (unsigned int LabeledMarkerIndex = 0; LabeledMarkerIndex < LabeledMarkerCount; ++LabeledMarkerIndex)
	  {
		  // Get the global marker translation
		  Output_GetLabeledMarkerGlobalTranslation _Output_GetLabeledMarkerGlobalTranslation =
			  MyClient.GetLabeledMarkerGlobalTranslation(LabeledMarkerIndex);


		  std::ofstream markerPos(markerPos, std::ios::app);
		  markerPos << (LabeledMarkerIndex);
		  markerPos << ('\t');
		  markerPos << (_Output_GetLabeledMarkerGlobalTranslation.Translation[0]);
		  markerPos << ('\t');
		  markerPos << (_Output_GetLabeledMarkerGlobalTranslation.Translation[1]);
		  markerPos << ('\t');
		  markerPos << (_Output_GetLabeledMarkerGlobalTranslation.Translation[2]);
		  markerPos << ('\n');


		  output_stream << "      Marker #" << LabeledMarkerIndex << ": ("
			  << _Output_GetLabeledMarkerGlobalTranslation.Translation[0] << ", "
			  << _Output_GetLabeledMarkerGlobalTranslation.Translation[1] << ", "
			  << _Output_GetLabeledMarkerGlobalTranslation.Translation[2] << ")" << std::endl;
	  }

	  MyClient.DisableSegmentData();
	  MyClient.DisableMarkerData();
	  MyClient.DisableUnlabeledMarkerData();
	  MyClient.DisableDeviceData();

	  // Disconnect and dispose
	  int t = clock();
	  std::cout << " Disconnecting..." << std::endl;
	  MyClient.Disconnect();
	  int dt = clock() - t;
	  double secs = (double)(dt) / (double)CLOCKS_PER_SEC;
	  std::cout << " Disconnect time = " << secs << " secs" << std::endl;
	  }
  }
}