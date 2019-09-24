#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "opencv2/opencv.hpp"

#define TAG_DEBUG_PERFORMANCE 0
#define TAG_DEBUG_DRAW 0
#include "apriltag/apriltag.hpp"
#include "apriltag/TagFamilyFactory.hpp"
#include "Singleton.hpp"

namespace py_apriltag {

using namespace std;
using namespace cv;
using april::tag::TagFamily;
using april::tag::TagFamilyFactory;
using april::tag::TagDetector;
using april::tag::TagDetection;

struct AprilTagDetector {
    std::vector< cv::Ptr<TagFamily> > gTagFamilies;
    cv::Ptr<TagDetector> gDetector;

	virtual ~AprilTagDetector() {}
	AprilTagDetector(string tagid="4") { //defaul Tag36h11
        this->set_tagfamilies(tagid, false);
	}

	void set_tagfamilies(string tagid, bool verbose) { //example input: "1", "12", "14", etc.
	    TagFamilyFactory::create(tagid, this->gTagFamilies);
        this->gDetector = new TagDetector(this->gTagFamilies);
        gDetector->segDecimate = false;
        if(gDetector.empty()) {
            throw std::runtime_error("create TagDetector fail!");
        }
        if(!verbose) return;
        cout << "Selected TagFamily:" << endl;
        for(int i=0; i<(int)this->gTagFamilies.size(); ++i) {
            cout << this->gTagFamilies[i]->familyName() << endl;
        }
        cout << "-------------------" << endl;
	}
	inline void set_tagfamilies(string tagid) {
	    set_tagfamilies(tagid, true);
	}

	size_t process(//return N detections with hamming distance <= hammingThresh
	    Mat& frame,             //HxW or HxWx3
	    vector<int>& ids,       //Nx1, detected tag ids
	    vector<double>& corners,//Nx4x2
	    vector<double>& centers,//Nx2
	    vector<double>& Hs,     //Nx3x3
	    int hammingThresh = 0
	)
	{
	    Mat frame_grey, frame_rgb;
	    bool do_vis = false;
	    if(frame.channels()==3) {
	        cvtColor(frame, frame_grey, CV_BGR2GRAY);
	        frame_rgb = frame;
	        do_vis = true;
	    } else {
	        frame_grey = frame;
	        //cvtColor(frame, frame_rgb, CV_GRAY2BGR);
	    }
	    vector<TagDetection> detections;
		gDetector->process(frame_grey, detections);

		ids.reserve(detections.size());
		corners.reserve(detections.size() * 4 * 2);
		centers.reserve(detections.size() * 2);
		Hs.reserve(detections.size() * 9);

		for (int i = 0; i < (int) detections.size(); ++i) {
			const TagDetection &dd = detections[i];
			if (dd.hammingDistance > hammingThresh)
				continue;

			ids.push_back(dd.id);

			for (int k = 0; k < 4; ++k) { //each marker <=> 4 corners
			    corners.push_back(dd.p[k][0]);
			    corners.push_back(dd.p[k][1]);
			}

			centers.push_back(dd.cxy[0]);
			centers.push_back(dd.cxy[1]);

			for (int ii=0; ii<3; ++ii) {
			    for (int jj=0; jj<3; ++jj) {
			        Hs.push_back(dd.homography[ii][jj]);
			    }
			}

			if(do_vis) {
			    cv::putText( frame_rgb, dd.toString(), cv::Point((int)dd.cxy[0],(int)dd.cxy[1]),
				         CV_FONT_NORMAL, 0.5, helper::CV_BLUE, 1 );
				cv::Mat Homo( 3, 3, CV_64FC1, detections[i].homography[0] );
                helper::drawHomography(frame_rgb, Homo);
                cv::circle(frame_rgb, cv::Point2d(dd.p[0][0],dd.p[0][1]), 3, helper::CV_GREEN, 2);
			}
		}

		if(do_vis) frame = frame_rgb;
		return ids.size();
	}
};//end of struct AprilTagDetector

typedef helper::Singleton<AprilTagDetector> GAprilTagDetector;

void set_tagfamilies(std::string tagid)
{
    GAprilTagDetector::Instance().set_tagfamilies(tagid);
}

size_t detect_apriltags(//return N detections with hamming distance <= hammingThresh
    Mat& frame,             //HxW
    vector<int>& ids,       //Nx1, detected tag ids
    vector<double>& corners,//Nx4x2
    vector<double>& centers,//Nx2
    vector<double>& Hs,     //Nx3x3
    const int hammingThresh=0
)
{
    return GAprilTagDetector::Instance().process(
        frame,
        ids, corners, centers, Hs,
        hammingThresh
    );
}

}//py_apriltag