

premake4 gmake win32


call d:\MinGW\mingwvars
call build_extern
cd projects
mingw32-make pbr_clnt pbr_svr pbr_ctrl
mingw32-make config=release pbr_clnt pbr_svr pbr_ctrl
cd ..
