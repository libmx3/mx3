{ 'targets': [
    {
        'target_name': 'leveldb',
        'type': 'static_library',
        'sources': [
            "<!@(find leveldb/db    -name '*.[c|h]' -o -name '*.hpp' -o -name '*.cpp' -o -name '*.cc')",
            "<!@(find leveldb/util  -name '*.[c|h]' -o -name '*.hpp' -o -name '*.cpp' -o -name '*.cc')",
            "<!@(find leveldb/table -name '*.[c|h]' -o -name '*.hpp' -o -name '*.cpp' -o -name '*.cc')",
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
        # disable sign compare warnings, since leveldb has a few of them
        'cflags_cc': [
            '-Wno-sign-compare',
            '-Wno-unused-function',
        ],
        'xcode_settings': {
            'OTHER_CPLUSPLUSFLAGS': ['<@(_cflags_cc)'],
        },
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
