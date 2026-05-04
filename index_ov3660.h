#pragma once

/*
 * Para este proyecto, la vista full es única para OV2640/OV3660.
 * Se mantiene este alias para evitar romper includes antiguos.
 */

#include <stddef.h>
#include "index_ov2640.h"

const uint8_t *const index_ov3660_html = index_ov2640_html;
const size_t index_ov3660_html_len = index_ov2640_html_len;
