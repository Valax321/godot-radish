# Ported from the gdbuild files

from SCons.Script import ARGUMENTS

extra_suffix = "r3d"
deprecated = "no"

# Set render backends appropriately
d3d12 = "no"
if ARGUMENTS.get("platform") == "macos":
    vulkan = "no"
opengl3 = "no"
forward_mobile_renderer = "no"

# Big global overrides (mostly turning off 2d components we don't use)
disable_advanced_gui = "yes"
disable_navigation_2d = "yes"
disable_physics_2d = "yes"
disable_xr = "yes"

# Modules
brotli = "no"
graphite = "no"
module_camera_enabled = "no"
module_enet_enabled = "no"
module_fbx_enabled = "no"
module_gltf_enabled = "no"
module_godot_physics_3d_enabled = "no"
module_jpg_enabled = "no"
module_jsonrpc_enabled = "no"
module_lightmapper_rd_enabled = "no"
module_mbedtls_enabled = "no"
module_msdfgen_enabled = "no"
module_multiplayer_enabled = "no"
module_openxr_enabled = "no"
module_raycast_enabled = "no"
module_text_server_adv_enabled = "no"
module_text_server_fb_enabled = "yes"
module_upnp_enabled = "no"
module_vhacd_enabled = "no"
module_webrtc_enabled = "no"
module_websocket_enabled = "no"
module_xatlas_unwrap_enabled = "no"
module_zip_enabled = "no"
