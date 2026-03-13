# strip_playfab_secrets.cmake
# Usage: cmake -DSRC=<input> -DDST=<output> -P strip_playfab_secrets.cmake
#
# Copies playfab_dev.json while clearing secretKey and customId values
# so that game builds never ship with developer credentials.

file(READ "${SRC}" content)

# Replace secretKey value with empty string
string(REGEX REPLACE
    "\"secretKey\"[ \t]*:[ \t]*\"[^\"]*\""
    "\"secretKey\": \"\""
    content "${content}"
)

# Replace customId value with empty string
string(REGEX REPLACE
    "\"customId\"[ \t]*:[ \t]*\"[^\"]*\""
    "\"customId\": \"\""
    content "${content}"
)

file(WRITE "${DST}" "${content}")
