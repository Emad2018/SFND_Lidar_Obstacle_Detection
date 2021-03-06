// PCL lib Functions for processing point clouds

#include "processPointClouds.h"
#include <unordered_set>

// constructor:
template <typename PointT> ProcessPointClouds<PointT>::ProcessPointClouds() {}

// de-constructor:
template <typename PointT> ProcessPointClouds<PointT>::~ProcessPointClouds() {}

template <typename PointT>
void ProcessPointClouds<PointT>::numPoints(
    typename pcl::PointCloud<PointT>::Ptr cloud) {
  std::cout << cloud->points.size() << std::endl;
}

template <typename PointT>
std::pair<typename pcl::PointCloud<PointT>::Ptr,
          typename pcl::PointCloud<PointT>::Ptr>
ProcessPointClouds<PointT>::FilterCloud(
    typename pcl::PointCloud<PointT>::Ptr cloud, float filterRes,
    Eigen::Vector4f minPoint, Eigen::Vector4f maxPoint) {

  // Time segmentation process
  auto startTime = std::chrono::steady_clock::now();

  // TODO:: Fill in the function to do voxel grid point reduction and region
  // based filtering
  typename pcl::PointCloud<PointT>::Ptr cloud_filtered(
      new pcl::PointCloud<PointT>);
  pcl::VoxelGrid<PointT> sor;
  sor.setInputCloud(cloud);
  sor.setLeafSize(filterRes, filterRes, filterRes);
  sor.filter(*cloud_filtered);

  // typename pcl::CropBox<PointT>::Ptr ROI(new pcl::CropBox<PointT>(true));
  pcl::CropBox<PointT> ROI(true);
  typename pcl::PointCloud<PointT>::Ptr ROI_filtered(
      new pcl::PointCloud<PointT>);

  ROI.setInputCloud(cloud_filtered);
  ROI.setMin(minPoint);
  ROI.setMax(maxPoint);
  ROI.filter(*ROI_filtered);

  std::vector<int> indices;
  typename pcl::PointCloud<PointT>::Ptr roof_ROI(new pcl::PointCloud<PointT>);
  pcl::CropBox<PointT> roof(true);
  roof.setInputCloud(ROI_filtered);
  roof.setMin(Eigen::Vector4f(-3, -2, -1, 1));
  roof.setMax(Eigen::Vector4f(3, 2, 1, 1));
  roof.filter(indices);
  roof.filter(*roof_ROI);

  pcl::PointIndices::Ptr inliers(new pcl::PointIndices);

  for (int index : indices) {
    inliers->indices.push_back(index);
  }

  pcl::ExtractIndices<PointT> extract;

  extract.setInputCloud(ROI_filtered);
  extract.setIndices(inliers);
  extract.setNegative(true);
  extract.filter(*ROI_filtered);

  auto endTime = std::chrono::steady_clock::now();
  auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
      endTime - startTime);
  std::cout << "filtering took " << elapsedTime.count() << " milliseconds"
            << std::endl;

  std::pair<typename pcl::PointCloud<PointT>::Ptr,
            typename pcl::PointCloud<PointT>::Ptr>
      segResult(roof_ROI, ROI_filtered);

  return segResult;
}

template <typename PointT>
std::pair<typename pcl::PointCloud<PointT>::Ptr,
          typename pcl::PointCloud<PointT>::Ptr>
ProcessPointClouds<PointT>::SeparateClouds(
    pcl::PointIndices::Ptr inliers,
    typename pcl::PointCloud<PointT>::Ptr cloud) {
  // TODO: Create two new point clouds, one cloud with obstacles and other with
  // segmented plane
  typename pcl::PointCloud<PointT>::Ptr obstCloud(
      new pcl::PointCloud<PointT>());
  typename pcl::PointCloud<PointT>::Ptr planeCloud(
      new pcl::PointCloud<PointT>());
  pcl::ExtractIndices<PointT> extract;

  for (int index : inliers->indices)
    planeCloud->points.push_back(cloud->points[index]);

  extract.setInputCloud(cloud);
  extract.setIndices(inliers);
  extract.setNegative(true);
  extract.filter(*obstCloud);
  std::pair<typename pcl::PointCloud<PointT>::Ptr,
            typename pcl::PointCloud<PointT>::Ptr>
      segResult(obstCloud, planeCloud);
  return segResult;
}

template <typename PointT>
std::pair<typename pcl::PointCloud<PointT>::Ptr,
          typename pcl::PointCloud<PointT>::Ptr>
ProcessPointClouds<PointT>::SegmentPlane(
    typename pcl::PointCloud<PointT>::Ptr cloud, int maxIterations,
    float distanceThreshold) {
  // Time segmentation process
  auto startTime = std::chrono::steady_clock::now();

  // TODO:: Fill in this function to find inliers for the cloud.

  pcl::ModelCoefficients::Ptr coefficients(new pcl::ModelCoefficients());
  pcl::PointIndices::Ptr inliers(new pcl::PointIndices());
  pcl::SACSegmentation<PointT> seg;
  // Optional
  seg.setOptimizeCoefficients(true);
  // Mandatory
  seg.setModelType(pcl::SACMODEL_PLANE);
  seg.setMethodType(pcl::SAC_RANSAC);
  seg.setMaxIterations(maxIterations);
  seg.setDistanceThreshold(distanceThreshold);
  seg.setInputCloud(cloud);
  seg.segment(*inliers, *coefficients);

  auto endTime = std::chrono::steady_clock::now();
  auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
      endTime - startTime);
  std::cout << "plane segmentation took " << elapsedTime.count()
            << " milliseconds" << std::endl;

  std::pair<typename pcl::PointCloud<PointT>::Ptr,
            typename pcl::PointCloud<PointT>::Ptr>
      segResult = SeparateClouds(inliers, cloud);
  return segResult;
}

template <typename PointT>
std::pair<typename pcl::PointCloud<PointT>::Ptr,
          typename pcl::PointCloud<PointT>::Ptr>
ProcessPointClouds<PointT>::Ransac_3D(
    typename pcl::PointCloud<PointT>::Ptr cloud, int maxIterations,
    float distanceThreshold) {

  std::unordered_set<int> inliersResult;
  srand(time(NULL));

  // TODO: Fill in this function
  auto startTime = std::chrono::steady_clock::now();
  // For max iterations
  while (maxIterations--) {
    // Randomly sample subset and fit line
    std::unordered_set<int> inliers;
    float x1, y1, z1, x2, y2, z2, x3, y3, z3 = 0;
    if (cloud->points.size() > 0) {

      while (inliers.size() < 3) {

        inliers.insert(rand() % (cloud->points.size()));
      }

      auto itr = inliers.begin();
      x1 = cloud->points[*itr].x;
      y1 = cloud->points[*itr].y;
      z1 = cloud->points[*itr].z;
      itr++;
      x2 = cloud->points[*itr].x;
      y2 = cloud->points[*itr].y;
      z2 = cloud->points[*itr].z;
      itr++;
      x3 = cloud->points[*itr].x;
      y3 = cloud->points[*itr].y;
      z3 = cloud->points[*itr].z;
    }
    // std::cout << "point#" << 50 - maxIterations << "\n";
    // std::cout << "(" << x1 << "," << y1 << "," << z1 << ")\n";
    // std::cout << "(" << x2 << "," << y2 << "," << z2 << ")\n";
    // std::cout << "(" << x3 << "," << y3 << "," << z3 << ")\n";

    float A = ((y2 - y1) * (z3 - z1)) - ((z2 - z1) * (y3 - y1));
    float B = ((z2 - z1) * (x3 - x1)) - ((x2 - x1) * (z3 - z1));
    float C = ((x2 - x1) * (y3 - y1)) - ((y2 - y1) * (x3 - x1));
    float D = -1 * ((A * x1) + (B * y1) + (C * z1));
    // Measure distance between every point and fitted line
    // If distance is smaller than threshold count it as inlier

    for (int index = 0; index < cloud->points.size(); index++) {

      if (inliers.count(index) > 0) {
        continue;
      }
      PointT point = cloud->points[index];
      float x = point.x;
      float y = point.y;
      float z = point.z;
      float d = abs(A * x + B * y + C * z + D) / sqrt(A * A + B * B + C * C);
      if (d < distanceThreshold) {
        inliers.insert(index);
      }
    }
    if (inliers.size() > inliersResult.size()) {
      inliersResult = inliers;
    }
  }

  typename pcl::PointCloud<PointT>::Ptr cloudInliers(
      new pcl::PointCloud<PointT>());
  typename pcl::PointCloud<PointT>::Ptr cloudOutliers(
      new pcl::PointCloud<PointT>());

  for (int index = 0; index < cloud->points.size(); index++) {
    PointT point = cloud->points[index];
    if (inliersResult.count(index))
      cloudInliers->points.push_back(point);
    else
      cloudOutliers->points.push_back(point);
  }
  auto endTime = std::chrono::steady_clock::now();
  auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
      endTime - startTime);
  std::cout << "Ransac_3D segmentation took " << elapsedTime.count()
            << " milliseconds" << std::endl;
  std::pair<typename pcl::PointCloud<PointT>::Ptr,
            typename pcl::PointCloud<PointT>::Ptr>
      segResult(cloudOutliers, cloudInliers);
  // Return indicies of inliers from fitted line with most inliers

  return segResult;
}

template <typename PointT>
std::vector<typename pcl::PointCloud<PointT>::Ptr>
ProcessPointClouds<PointT>::Clustering(
    typename pcl::PointCloud<PointT>::Ptr cloud, float clusterTolerance,
    int minSize, int maxSize) {

  // Time clustering process
  auto startTime = std::chrono::steady_clock::now();

  std::vector<typename pcl::PointCloud<PointT>::Ptr> clusters;

  // TODO:: Fill in the function to perform euclidean clustering to group
  // detected obstacles
  typename pcl::search::KdTree<PointT>::Ptr tree(
      new pcl::search::KdTree<PointT>);
  tree->setInputCloud(cloud);

  std::vector<pcl::PointIndices> cluster_indices;
  pcl::EuclideanClusterExtraction<PointT> ec;
  ec.setClusterTolerance(clusterTolerance); // 2cm
  ec.setMinClusterSize(minSize);
  ec.setMaxClusterSize(maxSize);
  ec.setSearchMethod(tree);
  ec.setInputCloud(cloud);
  ec.extract(cluster_indices);

  for (pcl::PointIndices getIndices : cluster_indices) {
    typename pcl::PointCloud<PointT>::Ptr cloud_cluster(
        new pcl::PointCloud<PointT>());
    for (int index : getIndices.indices) {
      cloud_cluster->points.push_back(cloud->points[index]);
    }
    cloud_cluster->width = cloud_cluster->size();
    cloud_cluster->height = 1;
    cloud_cluster->is_dense = true;
    clusters.push_back(cloud_cluster);
  }
  auto endTime = std::chrono::steady_clock::now();
  auto elapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(
      endTime - startTime);
  std::cout << "clustering took " << elapsedTime.count()
            << " milliseconds and found " << clusters.size() << " clusters"
            << std::endl;

  return clusters;
}

template <typename PointT>
Box ProcessPointClouds<PointT>::BoundingBox(
    typename pcl::PointCloud<PointT>::Ptr cluster) {

  // Find bounding box for one of the clusters
  PointT minPoint, maxPoint;
  pcl::getMinMax3D(*cluster, minPoint, maxPoint);

  Box box;
  box.x_min = minPoint.x;
  box.y_min = minPoint.y;
  box.z_min = minPoint.z;
  box.x_max = maxPoint.x;
  box.y_max = maxPoint.y;
  box.z_max = maxPoint.z;

  return box;
}

template <typename PointT>
void ProcessPointClouds<PointT>::savePcd(
    typename pcl::PointCloud<PointT>::Ptr cloud, std::string file) {
  pcl::io::savePCDFileASCII(file, *cloud);
  std::cerr << "Saved " << cloud->points.size() << " data points to " + file
            << std::endl;
}

template <typename PointT>
typename pcl::PointCloud<PointT>::Ptr
ProcessPointClouds<PointT>::loadPcd(std::string file) {

  typename pcl::PointCloud<PointT>::Ptr cloud(new pcl::PointCloud<PointT>);

  if (pcl::io::loadPCDFile<PointT>(file, *cloud) == -1) //* load the file
  {
    PCL_ERROR("Couldn't read file \n");
  }
  std::cerr << "Loaded " << cloud->points.size() << " data points from " + file
            << std::endl;

  return cloud;
}

template <typename PointT>
std::vector<boost::filesystem::path>
ProcessPointClouds<PointT>::streamPcd(std::string dataPath) {

  std::vector<boost::filesystem::path> paths(
      boost::filesystem::directory_iterator{dataPath},
      boost::filesystem::directory_iterator{});

  // sort files in accending order so playback is chronological
  sort(paths.begin(), paths.end());

  return paths;
}