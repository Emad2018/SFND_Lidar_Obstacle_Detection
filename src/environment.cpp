/* \author Aaron Brown */
// Create simple 3d highway enviroment using PCL
// for exploring self-driving car sensors

#include "processPointClouds.h"
#include "render/render.h"
#include "sensors/lidar.h"
// using templates for processPointClouds so also include .cpp to help linker
#include "processPointClouds.cpp"

std::vector<Car> initHighway(bool renderScene,
                             pcl::visualization::PCLVisualizer::Ptr &viewer) {

  Car egoCar(Vect3(0, 0, 0), Vect3(4, 2, 2), Color(0, 1, 0), "egoCar");
  Car car1(Vect3(15, 0, 0), Vect3(4, 2, 2), Color(0, 0, 1), "car1");
  Car car2(Vect3(8, -4, 0), Vect3(4, 2, 2), Color(0, 0, 1), "car2");
  Car car3(Vect3(-12, 4, 0), Vect3(4, 2, 2), Color(0, 0, 1), "car3");

  std::vector<Car> cars;
  cars.push_back(egoCar);
  cars.push_back(car1);
  cars.push_back(car2);
  cars.push_back(car3);

  if (renderScene) {
    renderHighway(viewer);
    egoCar.render(viewer);
    car1.render(viewer);
    car2.render(viewer);
    car3.render(viewer);
  }

  return cars;
}
void cityBlock(pcl::visualization::PCLVisualizer::Ptr &viewer,
               ProcessPointClouds<pcl::PointXYZI> *pointProcessorI,
               const pcl::PointCloud<pcl::PointXYZI>::Ptr &inputCloud) {
  // ----------------------------------------------------
  // -----Open 3D viewer and display City Block     -----
  // ----------------------------------------------------

  /*ProcessPointClouds<pcl::PointXYZI> *pointProcessorI =
      new ProcessPointClouds<pcl::PointXYZI>();
  pcl::PointCloud<pcl::PointXYZI>::Ptr inputCloud =
      pointProcessorI->loadPcd("../src/sensors/data/pcd/data_1/0000000000.pcd");
      */
  // renderPointCloud(viewer, inputCloud, "inputCloud");

  std::pair<pcl::PointCloud<pcl::PointXYZI>::Ptr,
            pcl::PointCloud<pcl::PointXYZI>::Ptr>
      fillter_Cloud =
          pointProcessorI->FilterCloud(inputCloud, .1f,
                                       Eigen::Vector4f(-30, -6, -3, 1),
                                       Eigen::Vector4f(30, 7, 1, 1));
  // renderPointCloud(viewer, fillter_Cloud.second, "inputCloud");
  Box roof_box = pointProcessorI->BoundingBox(fillter_Cloud.first);
  // renderBox(viewer, roof_box, 0, Color(1, 0, 1));

  ProcessPointClouds<pcl::PointXYZI>
      point_Processor; //(new ProcessPointClouds<pcl::PointXYZ>());

  std::pair<pcl::PointCloud<pcl::PointXYZI>::Ptr,
            pcl::PointCloud<pcl::PointXYZI>::Ptr>
      segmentCloud = point_Processor.Ransac_3D(fillter_Cloud.second, 20, 0.2);

  // renderPointCloud(viewer, segmentCloud.first, "obstCloud", Color(1, 0, 0));
  // renderPointCloud(viewer, segmentCloud.second, "planeCloud", Color(0, 1,
  // 0));

  std::vector<pcl::PointCloud<pcl::PointXYZI>::Ptr> cloudClusters =
      point_Processor.Clustering(segmentCloud.first, .7, 50, 600);

  // renderPointCloud(viewer, cloudClusters[0], "obstCloud", Color(1, 0, 0));

  int clusterId = 1;
  std::vector<Color> colors = {Color(1, 0, 0), Color(0, 1, 0), Color(0, 0, 1)};

  for (pcl::PointCloud<pcl::PointXYZI>::Ptr cluster : cloudClusters) {
    std::cout << "cluster size ";
    point_Processor.numPoints(cluster);
    renderPointCloud(viewer, cluster, "obstCloud" + std::to_string(clusterId),
                     colors[clusterId]);
    Box box = point_Processor.BoundingBox(cluster);
    renderBox(viewer, box, clusterId);
    ++clusterId;
  }
  renderPointCloud(viewer, segmentCloud.second, "planeCloud", Color(0, 1, 0));
}

void simpleHighway(pcl::visualization::PCLVisualizer::Ptr &viewer) {
  // ----------------------------------------------------
  // -----Open 3D viewer and display simple highway -----
  // ----------------------------------------------------

  // RENDER OPTIONS
  bool renderScene = false;
  std::vector<Car> cars = initHighway(renderScene, viewer);

  // TODO:: Create lidar sensor
  Lidar *lidar_sensor = new Lidar(cars, 0);
  pcl::PointCloud<pcl::PointXYZ>::Ptr point_cloud = lidar_sensor->scan();
  // renderRays(viewer, lidar_sensor->position, point_cloud);
  // renderPointCloud(viewer, point_cloud, "point_cloud");

  // TODO:: Create point processor
  ProcessPointClouds<pcl::PointXYZ>
      point_Processor; //(new ProcessPointClouds<pcl::PointXYZ>());
  std::pair<pcl::PointCloud<pcl::PointXYZ>::Ptr,
            pcl::PointCloud<pcl::PointXYZ>::Ptr>
      segmentCloud = point_Processor.SegmentPlane(point_cloud, 100, 0.2);
  // renderPointCloud(viewer, segmentCloud.first, "obstCloud", Color(1, 0, 0));
  // renderPointCloud(viewer, segmentCloud.second, "planeCloud", Color(0, 1,
  // 0));

  std::vector<pcl::PointCloud<pcl::PointXYZ>::Ptr> cloudClusters =
      point_Processor.Clustering(segmentCloud.first, 1.0, 3, 30);

  int clusterId = 0;
  std::vector<Color> colors = {Color(1, 0, 0), Color(0, 1, 0), Color(0, 0, 1)};

  for (pcl::PointCloud<pcl::PointXYZ>::Ptr cluster : cloudClusters) {
    std::cout << "cluster size ";
    point_Processor.numPoints(cluster);
    renderPointCloud(viewer, cluster, "obstCloud" + std::to_string(clusterId),
                     colors[clusterId]);
    Box box = point_Processor.BoundingBox(cluster);
    renderBox(viewer, box, clusterId);
    ++clusterId;
  }
}

// setAngle: SWITCH CAMERA ANGLE {XY, TopDown, Side, FPS}
void initCamera(CameraAngle setAngle,
                pcl::visualization::PCLVisualizer::Ptr &viewer) {

  viewer->setBackgroundColor(0, 0, 0);

  // set camera position and angle
  viewer->initCameraParameters();
  // distance away in meters
  int distance = 16;

  switch (setAngle) {
  case XY:
    viewer->setCameraPosition(-distance, -distance, distance, 1, 1, 0);
    break;
  case TopDown:
    viewer->setCameraPosition(0, 0, distance, 1, 0, 1);
    break;
  case Side:
    viewer->setCameraPosition(0, -distance, 0, 0, 0, 1);
    break;
  case FPS:
    viewer->setCameraPosition(-10, 0, 0, 0, 0, 1);
  }

  if (setAngle != FPS)
    viewer->addCoordinateSystem(1.0);
}

int main(int argc, char **argv) {
  std::cout << "starting enviroment" << std::endl;

  pcl::visualization::PCLVisualizer::Ptr viewer(
      new pcl::visualization::PCLVisualizer("3D Viewer"));
  CameraAngle setAngle = XY;
  initCamera(setAngle, viewer);
  // simpleHighway(viewer);
  ProcessPointClouds<pcl::PointXYZI> *pointProcessorI =
      new ProcessPointClouds<pcl::PointXYZI>();
  std::vector<boost::filesystem::path> stream =
      pointProcessorI->streamPcd("../src/sensors/data/pcd/data_1");
  auto streamIterator = stream.begin();
  pcl::PointCloud<pcl::PointXYZI>::Ptr inputCloudI;
  cityBlock(viewer, pointProcessorI, inputCloudI);

  while (!viewer->wasStopped()) {
    // Clear viewer
    viewer->removeAllPointClouds();
    viewer->removeAllShapes();

    // Load pcd and run obstacle detection process

    inputCloudI = pointProcessorI->loadPcd((*streamIterator).string());
    cityBlock(viewer, pointProcessorI, inputCloudI);

    streamIterator++;
    if (streamIterator == stream.end())
      streamIterator = stream.begin();

    viewer->spinOnce();
  }
}