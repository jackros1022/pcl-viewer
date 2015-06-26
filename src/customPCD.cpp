#include <pcl/point_cloud.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/io/pcd_io.h>

#define BYTE 8
/* Following Boilerplate largely copy pasta'd from pointclouds.org */
struct UberXYZ_RGB
{
  PCL_ADD_POINT4D;                  // preferred way of adding a XYZ+padding
  float rgb;
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW   // make sure our new allocators are aligned
} EIGEN_ALIGN16;                    // enforce SSE padding for correct memory alignment

POINT_CLOUD_REGISTER_POINT_STRUCT (UberXYZ_RGB,
                                   (float, x, x)
                                   (float, y, y)
                                   (float, z, z)
                                   (float, rgb, rgb)
)

// Pack bytes into float (allegedly helps with SSE performance)
void pack_rgb(UberXYZ_RGB *p, uint8_t rgb[3]){
	uint32_t rgb_mask = ((uint32_t)rgb[0] << 16 | (uint32_t)rgb[1] << 8 | (uint32_t)rgb[2]);
	p->rgb = *reinterpret_cast<float*>(&rgb_mask); // Type coursion int -> float
}

// Unpack routine TODO: break these into utils file
void unpack_rgb(UberXYZ_RGB *p, uint8_t (&rgb)[3]){
  uint32_t rgb_mask = *reinterpret_cast<uint32_t*>(&p->rgb);
  rgb[0] = (uint8_t)(rgb_mask >> 2 * BYTE);
  rgb[1] = (uint8_t)(rgb_mask >> BYTE);
  rgb[2] = (uint8_t)(rgb_mask);
}

int
main (int argc, char** argv)
{
  pcl::PointCloud<UberXYZ_RGB> cloud;
  cloud.points.resize (3);
  cloud.width = 3;
  cloud.height = 1;

  uint8_t red[3] = {255, 0, 0};
  uint8_t green[3] = {0, 255, 0};
  uint8_t blue[3] = {0, 0, 255};

// Lets test out our utils
  pack_rgb(&cloud.points[0] , red);
  pack_rgb(&cloud.points[1] , green);
  pack_rgb(&cloud.points[2] , blue);

  cloud.points[0].x = cloud.points[0].y = cloud.points[0].z = 0;
  cloud.points[1].x = cloud.points[1].y = cloud.points[1].z = 1;
  cloud.points[2].x = cloud.points[2].y = cloud.points[2].z = 2;

// Save the file
  pcl::io::savePCDFile ("pack_unpack.pcd", cloud);

// Clear cloud
  for(int pt=0; pt < cloud.size(); pt++)
    cloud.points[pt] = {0, 0, 0, 0};

// Load it back from disk
  pcl::io::loadPCDFile ("pack_unpack.pcd", cloud);

  for(int pt = 0; pt < 3; pt++){
    uint8_t rgb[3] = {0,0,0};
    unpack_rgb(&cloud.points[pt], rgb); // populate colors
    printf("Point %d: {%f, %f, %f} RGB:%f\tR| %d\tG| %d\t B| %d\n", pt,
      cloud.points[pt].x, cloud.points[pt].y, cloud.points[pt].z, cloud.points[pt].rgb,
      rgb[0], rgb[1], rgb[2]);
  }

}
