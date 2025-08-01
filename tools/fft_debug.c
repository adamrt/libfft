// This is mainly for debugging and testing purposes.
#include <stdio.h>

#define FFT_IMPLEMENTATION
#include "fft.h"

void read_geometry(void);

int main(void) {
    fft_init("../heretic/fft.bin");
    {
        read_geometry();
    }
    fft_shutdown();
}

void read_geometry(void) {
    fft_record_t records[40]; // reused buffer for reading records

    fft_io_entry_e map_ids[] = {
        F_MAP__MAP001_GNS,
        F_MAP__MAP002_GNS,
        F_MAP__MAP003_GNS,
        F_MAP__MAP004_GNS,
        F_MAP__MAP005_GNS,
        F_MAP__MAP006_GNS,
        F_MAP__MAP007_GNS,
        F_MAP__MAP008_GNS,
        F_MAP__MAP009_GNS,
        F_MAP__MAP010_GNS,
        F_MAP__MAP011_GNS,
        F_MAP__MAP012_GNS,
        F_MAP__MAP013_GNS,
        F_MAP__MAP014_GNS,
        F_MAP__MAP015_GNS,
        F_MAP__MAP016_GNS,
        F_MAP__MAP017_GNS,
        F_MAP__MAP018_GNS,
        F_MAP__MAP019_GNS,
        F_MAP__MAP020_GNS,
        F_MAP__MAP021_GNS,
        F_MAP__MAP022_GNS,
        F_MAP__MAP023_GNS,
        F_MAP__MAP024_GNS,
        F_MAP__MAP025_GNS,
        F_MAP__MAP026_GNS,
        F_MAP__MAP027_GNS,
        F_MAP__MAP028_GNS,
        F_MAP__MAP029_GNS,
        F_MAP__MAP030_GNS,
        F_MAP__MAP031_GNS,
        F_MAP__MAP032_GNS,
        F_MAP__MAP033_GNS,
        F_MAP__MAP034_GNS,
        F_MAP__MAP035_GNS,
        F_MAP__MAP036_GNS,
        F_MAP__MAP037_GNS,
        F_MAP__MAP038_GNS,
        F_MAP__MAP039_GNS,
        F_MAP__MAP040_GNS,
        F_MAP__MAP041_GNS,
        F_MAP__MAP042_GNS,
        F_MAP__MAP043_GNS,
        F_MAP__MAP044_GNS,
        F_MAP__MAP045_GNS,
        F_MAP__MAP046_GNS,
        F_MAP__MAP047_GNS,
        F_MAP__MAP048_GNS,
        F_MAP__MAP049_GNS,
        F_MAP__MAP050_GNS,
        F_MAP__MAP051_GNS,
        F_MAP__MAP052_GNS,
        F_MAP__MAP053_GNS,
        F_MAP__MAP054_GNS,
        F_MAP__MAP055_GNS,
        F_MAP__MAP056_GNS,
        F_MAP__MAP057_GNS,
        F_MAP__MAP058_GNS,
        F_MAP__MAP059_GNS,
        F_MAP__MAP060_GNS,
        F_MAP__MAP061_GNS,
        F_MAP__MAP062_GNS,
        F_MAP__MAP063_GNS,
        F_MAP__MAP064_GNS,
        F_MAP__MAP065_GNS,
        F_MAP__MAP066_GNS,
        F_MAP__MAP067_GNS,
        F_MAP__MAP068_GNS,
        F_MAP__MAP069_GNS,
        F_MAP__MAP070_GNS,
        F_MAP__MAP071_GNS,
        F_MAP__MAP072_GNS,
        F_MAP__MAP073_GNS,
        F_MAP__MAP074_GNS,
        F_MAP__MAP075_GNS,
        F_MAP__MAP076_GNS,
        F_MAP__MAP077_GNS,
        F_MAP__MAP078_GNS,
        F_MAP__MAP079_GNS,
        F_MAP__MAP080_GNS,
        F_MAP__MAP081_GNS,
        F_MAP__MAP082_GNS,
        F_MAP__MAP083_GNS,
        F_MAP__MAP084_GNS,
        F_MAP__MAP085_GNS,
        F_MAP__MAP086_GNS,
        F_MAP__MAP087_GNS,
        F_MAP__MAP088_GNS,
        F_MAP__MAP089_GNS,
        F_MAP__MAP090_GNS,
        F_MAP__MAP091_GNS,
        F_MAP__MAP092_GNS,
        F_MAP__MAP093_GNS,
        F_MAP__MAP094_GNS,
        F_MAP__MAP095_GNS,
        F_MAP__MAP096_GNS,
        F_MAP__MAP097_GNS,
        F_MAP__MAP098_GNS,
        F_MAP__MAP099_GNS,
        F_MAP__MAP100_GNS,
        F_MAP__MAP101_GNS,
        F_MAP__MAP102_GNS,
        F_MAP__MAP103_GNS,
        F_MAP__MAP104_GNS,
        F_MAP__MAP105_GNS,
        F_MAP__MAP106_GNS,
        F_MAP__MAP107_GNS,
        F_MAP__MAP108_GNS,
        F_MAP__MAP109_GNS,
        F_MAP__MAP110_GNS,
        F_MAP__MAP111_GNS,
        F_MAP__MAP112_GNS,
        F_MAP__MAP113_GNS,
        F_MAP__MAP114_GNS,
        F_MAP__MAP115_GNS,
        F_MAP__MAP116_GNS,
        F_MAP__MAP117_GNS,
        F_MAP__MAP118_GNS,
        F_MAP__MAP119_GNS,
        F_MAP__MAP125_GNS,
    };

    for (uint32_t i = 0; i < (sizeof(map_ids) / sizeof(map_ids[0])); i++) {
        fft_span_t gns = fft_io_open(map_ids[i]);
        uint32_t count = fft_record_read_all(&gns, records);
        fft_io_close(gns);

        for (uint8_t j = 0; j < count; j++) {
            fft_record_t primary = records[j];
            if (primary.type != FFT_RECORDTYPE_MESH_PRIMARY) {
                continue;
            }
            if (primary.sector == 0) {
                continue;
            }

            fft_span_t file = fft_io_read(primary.sector, primary.length);
            fft_mesh_t mesh = fft_mesh_read(&file);

            mesh.geometry.valid = true;
            fft_io_close(file);
        }
    }
}
