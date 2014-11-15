{ 'targets': [
    {
        'target_name': 'leveldb',
        'type': 'static_library',
        'sources': [
            "<!@(python ../glob.py leveldb/db *.c *.h *.hpp *.cpp *.cc)",
            "<!@(python ../glob.py leveldb/util  *.c *.h *.hpp *.cpp *.cc)",
            "<!@(python ../glob.py leveldb/table *.c *.h *.hpp *.cpp *.cc)",
            'leveldb/port/port_posix.cc',
        ],

        'sources/': [
            # exclude the testing and benchmarking sources
            ['exclude', '_test\\.c$'],
            ['exclude', '_main\\.cc$'],
            ['exclude', '_test\\.cc$'],
            ['exclude', '_bench\\.cc$'],
        ],

        'defines': [
            '_REENTRANT',
            'LEVELDB_PLATFORM_POSIX',
        ],
        'cflags': [
            # leveldb doesn't want memcmp on g++, we just are disabling it outright
            '-fno-builtin-memcmp',
            '-D_REENTRANT',
            '-DLEVELDB_PLATFORM_POSIX',
        ],
        'conditions': [
            ['OS=="mac"', {
                'defines+': ['OS_MACOSX'],
                'cflags+':  ['-DOS_MACOSX'],
            }],
            ['OS=="ios"', {
                # purposefully setting OS_MACOSX since that's how the leveldb build system wants it
                'defines+': ['OS_MACOSX'],
                'cflags+':  ['-DOS_MACOSX'],
            }],
            ['OS=="android"', {
                'defines+': ['OS_ANDROID'],
                'cflags+': ['-DOS_ANDROID'],
            }],
        ],
        'all_dependent_settings': {
            'include_dirs': [
                'leveldb/include',
            ]
        },
        'include_dirs': [
            'leveldb',
            'leveldb/include',
        ],
    },
]}
