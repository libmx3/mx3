{
    'configurations': {
      'Debug': {
        'xcode_settings': {
          'GCC_OPTIMIZATION_LEVEL': '0',
          'ONLY_ACTIVE_ARCH': 'YES',
        },
      },
    },

    'target_defaults': {
        'default_configuration': 'Debug',
        'cflags_cc': [ '-std=c++1y', '-fvisibility=hidden', '-fexceptions' ],
        'cflags': ['-Wall'],
        'xcode_settings': {
            'OTHER_CFLAGS' : ['-Wall', '-fvisibility=hidden'],
            'OTHER_CPLUSPLUSFLAGS' : ['-Wall', '-fvisibility=hidden'],
            'CLANG_CXX_LANGUAGE_STANDARD': 'c++1y',
            'CLANG_CXX_LIBRARY': 'libc++',
            'CLANG_ENABLE_OBJC_ARC': 'YES',
        },
        'conditions': [
            ['OS=="ios"', {
                'xcode_settings' : {
                    'SDKROOT': 'iphoneos',
                    'SUPPORTED_PLATFORMS': 'iphonesimulator iphoneos',
                },
            }],
            ['OS=="mac"', {
                'xcode_settings' : {
                    'SDKROOT': 'macosx10.8',
                },
            }],
        ],
        'configurations': {
            'Debug': {
                'defines': [ 'DEBUG' ],
                'cflags' : [ '-g', '-O0' ],
                'xcode_settings' : {
                },
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
