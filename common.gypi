{
    'target_defaults': {
        'default_configuration': 'Debug',
        'cflags_cc': [ '-std=c++11', '-fvisibility=hidden' ],
        'cflags': ['-Wall'],
        'xcode_settings': {
            'OTHER_CFLAGS' : ['-Wall', '-fvisibility=hidden'],
            'OTHER_CPLUSPLUSFLAGS' : ['-Wall', '-fvisibility=hidden'],
            'CLANG_CXX_LANGUAGE_STANDARD': 'c++11',
            'CLANG_CXX_LIBRARY': 'libc++',
        },
        'conditions': [
            ['OS=="ios"', {
                'xcode_settings' : {
                    'SDKROOT': 'iphoneos',
                    'SUPPORTED_PLATFORMS': 'iphonesimulator iphoneos',
                    'ARCHS': 'armv7 armv7s',
                },
            }],
        ],
        'configurations': {
            'Debug': {
                'defines': [ 'DEBUG' ],
                'cflags' : [ '-g', '-O0' ],
            },
            'Release': {
                'defines': [ 'NDEBUG' ],
                'cflags': [
                    '-Os',
                    '-fomit-frame-pointer',
                    '-fdata-sections',
                    '-ffunction-sections',
                ],
                'xcode_settings': {
                    'DEAD_CODE_STRIPPING': 'YES',
                },
            },
        },
    },
}
