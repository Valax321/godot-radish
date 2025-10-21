# Ported from the gdbuild files

from SCons.Script import ARGUMENTS

# Variables for configuration process
_radishPlatform = ARGUMENTS.get("platform")

# -----------------------------------------------------------------

extra_suffix = "2dbase" # Set your game identifier here
deprecated = "no"

# Use vulkan on all platforms...
# Assuming arm64-only macos builds (which might be fair enough since apple is killing intel support next year)
d3d12 = "no" # Yes, even on windows
if _radishPlatform == "macos" or _radishPlatform == "ios":
    vulkan = "no"
    metal = "yes"
else:
    vulkan = "yes"

# Note: I'm assuming here the use of the mobile renderer for a 2D game.
# You could disable the mobile renderer and rendering_device to just use the compat renderer (set opengl to yes then)
forward_plus_renderer = "no"
forward_mobile_renderer = "yes"
rendering_device = "yes"
opengl3 = "no"

# Big global overrides (mostly turning off 3d components we don't use)
disable_3d = "yes"
disable_advanced_gui = "yes" # Set to no if you need RichTextBox!!!!
disable_navigation_3d = "yes"
disable_physics_3d = "yes"
disable_xr = "yes"

# Modules!
# Some notes:
# We don't disable basis, bcdec, dds or ktx since we want to keep some texture compression around.
# We keep webp since that seems to be what godot uses internally for lossy compressed textures (even after import)
brotli = "no"
graphite = "no"
module_astcenc_enabled = "no"
module_betsy_enabled = "no"
module_bmp_enabled = "no"
module_camera_enabled = "no"
module_csg_enabled = "no"
module_enet_enabled = "no"
module_etcpak_enabled = "no"
module_fbx_enabled = "no"
module_gltf_enabled = "no"
module_godot_physics_3d_enabled = "no"
module_hdr_enabled = "no"
module_jolt_enabled = "no"
module_jpg_enabled = "no"
module_jsonrpc_enabled = "no"
module_lightmapper_rd_enabled = "no"
module_mbedtls_enabled = "no"
module_meshoptimizer_enabled = "no"
module_msdfgen_enabled = "no"
module_multiplayer_enabled = "no"
module_openxr_enabled = "no"
module_raycast_enabled = "no"
module_svg_enabled = "no"
module_text_server_adv_enabled = "no"
module_text_server_fb_enabled = "yes"
module_tga_enabled = "no"
module_tinyexr_enabled = "no"
module_upnp_enabled = "no"
module_vhacd_enabled = "no"
module_webrtc_enabled = "no"
module_websocket_enabled = "no"
module_xatlas_unwrap_enabled = "no"
module_zip_enabled = "no"
