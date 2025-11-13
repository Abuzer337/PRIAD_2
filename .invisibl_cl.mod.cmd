savedcmd_invisibl_cl.mod := printf '%s\n'   invisibl_cl.o | awk '!x[$$0]++ { print("./"$$0) }' > invisibl_cl.mod
