#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

#include "AllHelpers.h"
#include "apriltag/apriltag.hpp"
#include "apriltag/TagFamilyFactory.hpp"

namespace py_apriltag {

using namespace std;
using namespace cv;
using april::tag::UINT64;
using april::tag::TagFamily;
using april::tag::TagFamilyFactory;
using april::tag::TagDetector;
using april::tag::TagDetection;
using helper::ImageSource;

std::vector< cv::Ptr<TagFamily> > gTagFamilies;
cv::Ptr<TagDetector> gDetector;

/**
Calibration rig
To define Calibration rig in config file, simply write in AprilCalib.cfg as follows:
CalibRig={
	mode=3d
#names of each tag
	markerNames=[]
#repeat follow lines N times with i=S...S+N-1, (Xi, Yi, Zi) is the world coordinate
#of the center of the tagi
	tagCenters=[]
}
*/
struct CalibRig {
	bool isTag3d;

	typedef std::map<std::string, cv::Point3d> DataMap;
	DataMap name2Xw;

	CalibRig() : isTag3d(true)
	{}

	operator std::string() const {
		std::stringstream ss;
		ss<<"CalibRig::mode="<<(isTag3d?"3d":"2d")<<std::endl;
		ss<<"CalibRig::name2Xw=["<<std::endl;
		DataMap::const_iterator itr=name2Xw.begin();
		for(; itr!=name2Xw.end(); ++itr) {
			ss<<"\t"<<itr->first<<" "<<itr->second<<std::endl;
		}
		ss<<"]"<<std::endl;
		return ss.str();
	}

	void loadFromImage(string fname) {
	    cv::Mat img = cv::imread(fname, 0);
	    if(img.empty()) {
	        throw "cannot load image: "+fname;
	    }
	    vector<TagDetection> detections;
		gDetector->process(img, detections);
	    this->isTag3d = false;

	    int cnt=0;
	    for(int i=0; i<(int)detections.size(); ++i) {
			TagDetection &dd = detections[i];
			if(dd.hammingDistance>0) continue; //strict!

			name2Xw[dd.toString()]=cv::Point3d(dd.cxy[0],dd.cxy[1],0);
			++cnt;
		}
		logli("[CalibRig] loaded "<<cnt<<" tags.");
		if(cnt<=4)
		    throw "calib rig does not have enough tags than minimum (4)!";
	}

	bool name2worldPt(const std::string& name, double worldPt[3]) const {
		DataMap::const_iterator itr=name2Xw.find(name);
		if(itr==name2Xw.end()) return false;
		const cv::Point3d& pt=itr->second;
		worldPt[0]=pt.x;
		worldPt[1]=pt.y;
		worldPt[2]=pt.z;
		return true;
	}
} gRig;

struct AprilCalibprocessor : public ImageHelper::ImageSource::Processor {
	typedef std::map<std::string, cv::Point2f> String2Point2f;
	String2Point2f name2pt;

	double tagTextScale;
	int tagTextThickness;
	bool doLog;
	bool doAutoLog;
	int autoLogCountDown;
	float autoLogPixTh;
	int autoLogFreeze;
	bool isPhoto;
	bool useEachValidPhoto;
	bool logVisFrame;
	std::string outputDir;

	int nDistCoeffs;
	std::vector<std::vector<cv::Point2f> > imagePtsArr;
	std::vector<std::vector<cv::Point3f> > worldPtsArr;
	cv::Mat K, CovK;
	double rmsThresh;

	int AUTO_LOG_COUNTDOWN;
	int AUTO_LOG_FREEZE;

	virtual ~AprilCalibprocessor() {}
	AprilCalibprocessor() : doLog(false), autoLogFreeze(0), isPhoto(false) {
		tagTextScale = 1.0f;
		tagTextThickness = 1;
		useEachValidPhoto = false;
		rmsThresh = 2;
		nDistCoeffs = 2;
		logVisFrame = false;
		doAutoLog = true;
		autoLogPixTh = 10;
		autoLogCountDown = AUTO_LOG_COUNTDOWN = 10;
		AUTO_LOG_FREEZE = 30;
		if(nDistCoeffs>5) {
			nDistCoeffs=5;
			logli("[AprilCalibprocessor warn] AprilCalib::nDistCoeffs>5, set back to 5!");
		}
		gDetector->segDecimate = false;
		CovK = cv::Mat::eye(9,9,cv::DataType<double>::type) * 100;
	}
/////// Override
	void operator()(cv::Mat& frame) {
		if(autoLogFreeze>0) {
			--autoLogFreeze;
			return;
		}

		//1. process frame
		static helper::PerformanceMeasurer PM;
		vector<TagDetection> detections;
		cv::Mat orgFrame;
		frame.copyTo(orgFrame);
		PM.tic();
		gDetector->process(frame, detections);
		logld("[TagDetector] process time = "<<PM.toc()<<" sec.");

		std::vector<cv::Point3f> worldPts;
		std::vector<cv::Point2f> imagePts;
		worldPts.reserve(detections.size());
		imagePts.reserve(detections.size());

		bool allDetectionSteady = true, hasDetection = false;

		logld(">>> find: ");
		for(int i=0; i<(int)detections.size(); ++i) {
			TagDetection &dd = detections[i];
			if(dd.hammingDistance>0) continue; //strict!
			hasDetection = true;

			if(doAutoLog && !isPhoto && allDetectionSteady) {
				allDetectionSteady = !hasChangedMoreThan(dd, autoLogPixTh);
			}

			double worldPt[3];
			if(!gRig.name2worldPt(dd.toString(), worldPt)) {
				logli("[AprilCalibprocessor info] ignore tag with id="<<dd.id);
				continue;
			}
			worldPts.push_back(cv::Point3f((float)worldPt[0],(float)worldPt[1],(float)worldPt[2]));
			imagePts.push_back(cv::Point2f((float)dd.cxy[0], (float)dd.cxy[1]));

			//visualization
			logld("id="<<dd.id<<", hdist="<<dd.hammingDistance<<", rotation="<<dd.rotation);
			cv::putText( frame, dd.toString(), cv::Point((int)dd.cxy[0],(int)dd.cxy[1]),
				         CV_FONT_NORMAL, tagTextScale, helper::CV_BLUE, tagTextThickness );
			cv::Mat Homo = cv::Mat(3,3,CV_64FC1,dd.homography[0]);
			helper::drawHomography(frame, Homo);
			cv::circle(frame, cv::Point2d(dd.p[0][0],dd.p[0][1]), 3, helper::CV_GREEN, 2);
		}

		{//visualization
			cv::putText( frame,
				doAutoLog?
				cv::format("captured=%d, countdown=%d",imagePtsArr.size(),autoLogCountDown)
				:cv::format("captured=%d",imagePtsArr.size()),
				cv::Point(5,15), CV_FONT_NORMAL, 0.5, helper::CV_BLUE );

			cv::Mat ck;
			cv::sqrt(CovK.diag(), ck);
			cv::putText( frame,
				cv::format("sigma: fx=%.2f, fy=%.2f, cx=%.2f, cy=%.2f",
				ck.at<double>(0), ck.at<double>(1), ck.at<double>(2), ck.at<double>(3)),
				cv::Point(5,35), CV_FONT_NORMAL, 0.5, helper::CV_RED );
		}

		//2. do calibration
		if((worldPts.size()>=8 && gRig.isTag3d)
			|| (worldPts.size()>=4 && !gRig.isTag3d))
		{//TODO: need to judge whether is degenerated case (planar structure for 3d or line for 2d)
			if(!gRig.isTag3d && helper::cloudShape(worldPts)<=1) {
				autoLogCountDown=AUTO_LOG_COUNTDOWN;
				return;
			}
			if(doAutoLog && !isPhoto) {
				doLog = hasDetection && allDetectionSteady;
				updateName2Pt(detections);
				if(doLog) {
					--autoLogCountDown;
				} else {
					autoLogCountDown=AUTO_LOG_COUNTDOWN; //not captured, restart
				}
				if(autoLogCountDown==0) {
					autoLogCountDown=AUTO_LOG_COUNTDOWN; //to be captured, restart
					autoLogFreeze=AUTO_LOG_FREEZE;
				} else {
					doLog=false;
				}
			}
			addNewFrameAndCalib(frame, orgFrame, worldPts, imagePts);
		} else {
			if(doAutoLog && !isPhoto) {
				autoLogCountDown=AUTO_LOG_COUNTDOWN;
			}
		}
	}

	bool hasChangedMoreThan(const TagDetection& dd, const float pixTh) const {
		String2Point2f::const_iterator itr = name2pt.find(dd.name());
		if(itr==name2pt.end()) return false;
		const cv::Point2f& pt = itr->second;
		return std::abs<float>((float)dd.cxy[0]-pt.x)>pixTh
			|| std::abs<float>((float)dd.cxy[1]-pt.y)>pixTh;
	}

	void updateName2Pt(const vector<TagDetection>& detections) {
		name2pt.clear();
		for(size_t i=0; i<detections.size(); ++i) {
			const TagDetection &dd = detections[i];
			name2pt[dd.name()]=cv::Point2f((float)dd.cxy[0], (float)dd.cxy[1]);
		}
	}

	void addNewFrameAndCalib(
		const cv::Mat& frame,
		const cv::Mat& orgFrame,
		const std::vector<cv::Point3f>& worldPts,
		const std::vector<cv::Point2f>& imagePts)
	{
		cv::Mat Ut, Xwt, P;
		cv::Mat(imagePts).reshape(1).convertTo(Ut, cv::DataType<double>::type);
		cv::Mat(worldPts).reshape(1).convertTo(Xwt, cv::DataType<double>::type);
		if(gRig.isTag3d && this->K.empty()) {
			cv::Mat K0,Rwc,twc;
			helper::dlt3<double>(Ut.t(), Xwt.t(), P);
			helper::decomposeP10<double>(P, K0, Rwc, twc);
			logli("K_dlt="<<K0);
			K0.copyTo(this->K);
		}

		if(doLog || (isPhoto && useEachValidPhoto)) {
			doLog=false;
			static int cnt=0;
			std::ofstream ofs((outputDir+"AprilCalib_log_"+helper::num2str(cnt,5)+".py").c_str());
			ofs<<"# AprilCalib log "<<cnt<<std::endl;
			ofs<<"# CalibRig::mode="<<(gRig.isTag3d?"3d":"2d")<<std::endl;
			ofs<<"# @ "<<LogHelper::getCurrentTimeString()<<std::endl;
			ofs<<"from numpy import array"<<std::endl;
			ofs<<"U="<< cv::format(Ut.t(), cv::Formatter::FMT_NUMPY) <<";"<<std::endl;
			ofs<<"Xw="<< cv::format(Xwt.t(), cv::Formatter::FMT_NUMPY) <<";"<<std::endl;
			if(gRig.isTag3d) ofs<<"P="<<cv::format(P, cv::Formatter::FMT_NUMPY)<<";"<<std::endl;

			if(logVisFrame) cv::imwrite(outputDir+"AprilCalib_frame_"+helper::num2str(cnt,5)+".png", frame);
			if(!isPhoto) cv::imwrite(outputDir+"AprilCalib_orgframe_"+helper::num2str(cnt,5)+".png", orgFrame);
			++cnt;

			this->imagePtsArr.push_back(imagePts);
			this->worldPtsArr.push_back(worldPts);
			if(cnt>=2) {
				cv::Mat distCoeffs;
				if(this->nDistCoeffs>0) {
					distCoeffs=cv::Mat::zeros(nDistCoeffs,1,CV_64FC1);
				}
				std::vector<cv::Mat> rvecs, tvecs, Covrs, Covts;
				double rms=0;
				helper::intrinsicCalibration(imagePtsArr, worldPtsArr,
					gRig.isTag3d,
					cv::Size(frame.cols, frame.rows), K, distCoeffs,
					rvecs, tvecs, rms, &CovK, &Covrs, &Covts);

				if(rms>rmsThresh) { //rms too large usually means gross error
					logli("[AprilCalib warn] rms="<<rms<<", too large, ignore this image.");
					--cnt;
					doLog=true;
					this->imagePtsArr.pop_back();
					this->worldPtsArr.pop_back();
				} else {
					ofs<<"# After LM:"<<std::endl;
					ofs<<"K="<<cv::format(K, cv::Formatter::FMT_NUMPY)<<";"<<std::endl;
					ofs<<"distCoeffs="<<cv::format(distCoeffs, cv::Formatter::FMT_NUMPY)<<";"<<std::endl;
					ofs<<"CovK="<<cv::format(CovK, cv::Formatter::FMT_NUMPY)<<";"<<std::endl;
					ofs<<"# rms="<<rms<<std::endl;
					for(int i=0; i<(int)rvecs.size(); ++i) {
						ofs<<"r"<<i<<"="<<cv::format(rvecs[i], cv::Formatter::FMT_NUMPY)<<";"<<std::endl;
						ofs<<"t"<<i<<"="<<cv::format(tvecs[i], cv::Formatter::FMT_NUMPY)<<";"<<std::endl;
						ofs<<"Covr"<<i<<"="<<cv::format(Covrs[i], cv::Formatter::FMT_NUMPY)<<";"<<std::endl;
						ofs<<"Covt"<<i<<"="<<cv::format(Covts[i], cv::Formatter::FMT_NUMPY)<<";"<<std::endl;
					}
				}//if rms>rmsThresh
			}//if cnt>=2
			ofs.close();
		}//if doLog
	}

	void handle(char key) {
		switch (key) {
		case 'd':
			gDetector->segDecimate = !(gDetector->segDecimate);
			logli("[ProcessVideo] detector.segDecimate="<<gDetector->segDecimate); break;
		case '1':
			LogHelper::GetOrSetLogLevel(LogHelper::LOG_DEBUG); break;
		case '2':
			LogHelper::GetOrSetLogLevel(LogHelper::LOG_INFO); break;
		case 'l':
			doLog=true; break;
		case 'h':
			cout<<"d: segDecimate\n"
				"l: do log\n"
				"1: debug output\n"
				"2: info output\n"<<endl; break;
		}
	}

};//end of struct AprilCalibprocessor

int calib_by_apriltags(
    string rig_filename, string url="camera://0", string log_dir="",
    int nDistCoeffs=2, bool useEachValidPhoto=true)
{
	LogHelper::GetOrSetLogLevel(LogHelper::LOG_INFO);

	cv::Ptr<ImageSource> is = helper::createImageSource(url);
	if(is.empty()) {
		tagle("createImageSource failed!");
		return -1;
	}
	is->reportInfo();

	//// create tagFamily
	string tagid = "4";
	gTagFamilies.clear();
	TagFamilyFactory::create(tagid, gTagFamilies);
	if(gTagFamilies.size()<=0) {
		tagle("create TagFamily failed all! exit...");
		return -1;
	}

	gDetector = new TagDetector(gTagFamilies);
	if(gDetector.empty()) {
		tagle("create TagDetector fail!");
		return -1;
	}

	gRig.loadFromImage(rig_filename);
	tagli("the Calibration Rig is:\n"<<std::string(gRig));
	AprilCalibprocessor processor;
	processor.nDistCoeffs = nDistCoeffs;
	processor.isPhoto = is->isClass<helper::ImageSource_Photo>();
	if (processor.isPhoto) {
	    processor.useEachValidPhoto = useEachValidPhoto;
	    is->loop(false);
	}
	if(log_dir.empty())
	    processor.outputDir = is->getSourceDir();
	else
	    processor.outputDir = log_dir;
	helper::legalDir( processor.outputDir );
	logli("[main] detection will be logged to outputDir="<<processor.outputDir);
	is->run(processor,-1, false,
			is->getPause(),
			is->getLoop() );

	cout<<"[main] DONE...exit!"<<endl;
	return 0;
}

}//py_apriltag