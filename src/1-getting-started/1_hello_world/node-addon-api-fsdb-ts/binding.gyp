{
  "targets": [
    {
      "target_name": "readFsdb",
      "cflags!": [ "-fno-exceptions" ],
      "cflags_cc!": [ "-fno-exceptions" ],
      "sources": [ "read_fsdb.cc" ],
      "include_dirs": [
        "<!@(node -p \"require('node-addon-api').include\")",
        "/home/heyfey/verdi/2022.06/share/FsdbReader_build"
      ],
      'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
      "libraries": [
        "/home/heyfey/verdi/2022.06/share/FsdbReader_build/linux64/libnffr.so",
        "/home/heyfey/verdi/2022.06/share/FsdbReader_build/linux64/libnsys.so",
      ],
    }
  ]
}
