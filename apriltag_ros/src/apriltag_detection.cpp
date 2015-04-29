#include "apriltag_ros/apriltag_detection.h"

#include <opencv2/highgui/highgui.hpp>
#include "sv_base/math.h"

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
    apriltag.corners[i] = point;
    if (size > 0) {
      apriltag.pose.position.x = t(0);
      apriltag.pose.position.y = t(1);
      apriltag.pose.position.z = t(2);
      apriltag.pose.orientation.w = q.w();
      apriltag.pose.orientation.x = q.x();
      apriltag.pose.orientation.y = q.y();
      apriltag.pose.orientation.z = q.z();
    }
  }

  return apriltag;
}

void ApriltagDetection::estimate(const cv::Matx33d& K, double tag_size) {
  assert(tag_size > 0);
  size = tag_size;
  const auto s = tag_size / 2.0;

  // tag corners in tag frame
  std::vector<cv::Point3d> p_tag = {
      {-s, -s, 0}, {s, -s, 0}, {s, s, 0}, {-s, s, 0}};

  // pixels coordinates in image frame
  std::vector<cv::Point2d> p_img = {{p[0][0], p[0][1]},
                                    {p[1][0], p[1][1]},
                                    {p[2][0], p[2][1]},
                                    {p[3][0], p[3][1]}};

  // Estimate r and t
  // TODO: can not use Matx type here?
  cv::Mat rvec, tvec;
  // Assume rectified image, so distortion is just 0s
  const cv::Mat_<double> D(1, 4, 0.0);
  cv::solvePnP(p_tag, p_img, K, D, rvec, tvec);
  t = Eigen::Vector3d(tvec.at<double>(0), tvec.at<double>(1),
                      tvec.at<double>(2));
  Eigen::Vector3d r(rvec.at<double>(0), rvec.at<double>(1), rvec.at<double>(2));

  // Convert r to quat
  //  const auto angle = r.norm();
  //  Eigen::Vector3d axis(0, 0, 0);
  //  if (angle > std::numeric_limits<double>::epsilon() * 10) {
  //    axis = r / angle;
  //  }
  //  q = Eigen::AngleAxis<double>(angle, axis);
  q = sv::base::RotationVectorToQuaternion(r);
}

void ApriltagDetection::draw(cv::Mat& image, int thickness) const {
  drawLine(image, 0, 1, CV_RED, thickness);
  drawLine(image, 0, 3, CV_GREEN, thickness);
  drawLine(image, 2, 3, CV_BLUE, thickness);
  drawLine(image, 1, 2, CV_BLUE, thickness);

  cv::putText(image, std::to_string(id), cv::Point2f(c[0] - 5, c[1] + 5),
              cv::FONT_HERSHEY_SIMPLEX, 1, CV_MAGENTA, 2);
}

void ApriltagDetection::drawLine(cv::Mat& image, int b, int e,
                                 const cv::Scalar& color, int thickness) const {
  cv::line(image, cv::Point2f(p[b][0], p[b][1]), cv::Point2f(p[e][0], p[e][1]),
           color, thickness);
}

}  // apriltag_ros
