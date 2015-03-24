#include "apriltag_ros/apriltag_detection.h"

#include <opencv2/highgui/highgui.hpp>

#define CV_RED CV_RGB(255, 0, 0)
#define CV_GREEN CV_RGB(0, 255, 0)
#define CV_BLUE CV_RGB(0, 0, 255)
#define CV_MAGENTA CV_RGB(255, 0, 255)

namespace apriltag_ros {

ApriltagDetection::ApriltagDetection(const AprilTags::TagDetection& td)
    : id(td.id), hamming(td.hammingDistance) {
  c[0] = td.cxy.first;
  c[1] = td.cxy.second;
  for (size_t i = 0; i < 4; ++i) {
    p[i][0] = td.p[i].first;
    p[i][1] = td.p[i].second;
  }
}

ApriltagDetection::ApriltagDetection(const apriltag_detection_t* td)
    : id(td->id), hamming(td->hamming) {
  c[0] = td->c[0];
  c[1] = td->c[1];
  ///@note: the order is different
  for (size_t i = 0; i < 4; ++i) {
    p[i][0] = td->p[3 - i][0];
    p[i][1] = td->p[3 - i][1];
  }
}

ApriltagDetection::operator apriltag_msgs::Apriltag() const {
  apriltag_msgs::Apriltag apriltag;
  apriltag.id = id;
  apriltag.hamming = hamming;
  apriltag.center.x = c[0];
  apriltag.center.y = c[1];
  for (size_t i = 0; i < 4; ++i) {
    geometry_msgs::Point point;
    point.x = p[i][0];
    point.y = p[i][1];
    point.z = 1;
    apriltag.corners.push_back(point);
  }

  return apriltag;
}

void ApriltagDetection::Draw(cv::Mat& image, int thickness) const {
  DrawLine(image, 0, 1, CV_RED, thickness);
  DrawLine(image, 0, 3, CV_GREEN, thickness);
  DrawLine(image, 2, 3, CV_BLUE, thickness);
  DrawLine(image, 1, 2, CV_BLUE, thickness);

  cv::putText(image, std::to_string(id), cv::Point2f(c[0] - 5, c[1] + 5),
              cv::FONT_HERSHEY_SIMPLEX, 1, CV_MAGENTA, 2);
}

void ApriltagDetection::DrawLine(cv::Mat& image, int b, int e,
                                 const cv::Scalar& color, int thickness) const {
  cv::line(image, cv::Point2f(p[b][0], p[b][1]), cv::Point2f(p[e][0], p[e][1]),
           color, thickness);
}

}  // apriltag_ros
