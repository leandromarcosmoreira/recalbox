#include <fcntl.h>
#include <kms-mode.h>
#include <libdrm/drm.h>
#include <stdio.h>
#include <unistd.h>
#include <xf86drmMode.h>

char *cards[3] = {"/dev/dri/card1", "/dev/dri/card0", "/dev/dri/card2"};


bool checkConnector(int fd, drmModeConnectorPtr connector, video_mode *res_mode) {
  if (connector->connection == DRM_MODE_CONNECTED) {
    drmModeEncoderPtr encoder = drmModeGetEncoder(fd, connector->encoder_id);
    printf("current crtc :%d\n", encoder->crtc_id);
    drmModeCrtcPtr crct = drmModeGetCrtc(fd, encoder->crtc_id);
    drmModeModeInfo mode = crct->mode;
    printf("current mode : %s\n", crct->mode.name);
    printf("current mode :\nvrefresh : %d\nvdisplay : %d\nvscan : %d\nvtotal : %d\n",
           crct->mode.vrefresh, crct->mode.vdisplay, crct->mode.vscan, crct->mode.vtotal);
    printf("current mode :\nhdisplay : %d\nhskew : %d\n", crct->mode.hdisplay, crct->mode.hskew);
    printf("current mode :\ntype : %d\nclock : %d\n", crct->mode.type, crct->mode.clock);
    printf("current mode :\nvsync : %d\nclock : %d\n", crct->mode.type, crct->mode.clock);
    float vrefresh = (mode.clock * 1000.0) / (mode.htotal * mode.vtotal) *
                     ((mode.flags & DRM_MODE_FLAG_INTERLACE) ? 2 : 1);
    unsigned int hrefresh = mode.clock * 1000.0 / mode.htotal;
    printf("current vsync : %f\n", vrefresh);
    printf("current hsync : %d\n", hrefresh);
    res_mode->hfreq = hrefresh;
    res_mode->vfreq = vrefresh;
    res_mode->width = mode.hdisplay;
    res_mode->height = mode.vdisplay;
    res_mode->interlaced = ((mode.flags & DRM_MODE_FLAG_INTERLACE) ? 1 : 0);
    drmModeFreeCrtc(crct);
    drmModeFreeEncoder(encoder);
    return true;
  }
  return false;
}

video_mode get_current_mode() {
  video_mode res_mode = {
      .width = 0,
      .height = 0,
      .interlaced = 0,
      .vfreq = 0,
      .hfreq = 0,
  };

  bool found = false;

  for (int card = 0; card < 3; card++) {
    if (!found) {
      int fd = open(cards[card], O_RDWR);
      if(fd == -1) continue;
      drmModeResPtr resource = drmModeGetResources(fd);

      if (resource != NULL) {
        printf("Open card with %d connectors", resource->count_connectors);
        for (int connectorIndex = 0; connectorIndex < resource->count_connectors; ++connectorIndex) {
          drmModeConnectorPtr connector = drmModeGetConnector(fd, resource->connectors[connectorIndex]);
          if (connector != NULL) {
            printf("Opening connector %d (%s) (%d)\n", connectorIndex,
                   (connector->connection == DRM_MODE_CONNECTED ? "Connected" : "Disconnected"),
                   resource->connectors[connectorIndex]);
            if (checkConnector(fd, connector, &res_mode)) {
              drmModeFreeConnector(connector);
              found = true;
              break;
            }
          }
          drmModeFreeConnector(connector);
        }
      }
      drmModeFreeResources(resource);
      close(fd);
    }
  }
  return res_mode;
}
