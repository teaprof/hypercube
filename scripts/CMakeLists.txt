#configure_file(generate_basic.sh FILE_PERMISSIONS "r-xr-xr-x" COPYONLY)
set(permissions OWNER_EXECUTE GROUP_EXECUTE OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE WORLD_READ WORLD_EXECUTE)
configure_file(generate_basic.sh generate_basic.sh FILE_PERMISSIONS ${permissions} COPYONLY)