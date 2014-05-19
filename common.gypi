{
    'target_defaults': {
        'default_configuration': 'Debug',
        'cflags_cc': [ '-std=c++11', '-fvisibility=hidden' ],
        'cflags': ['-Wall'],
        'ldflags': ['-stdlib=libc++'],
        'xcode_settings': {
            'OTHER_CFLAGS' : ['-Wall', '-fvisibility=hidden'],
            'OTHER_CPLUSPLUSFLAGS' : ['-Wall', '-fvisibility=hidden'],
            'CLANG_CXX_LANGUAGE_STANDARD': 'c++11',
            'CLANG_CXX_LIBRARY': 'libc++',
            'CLANG_ENABLE_OBJC_ARC': 'YES',
        },
        'conditions': [
            ['OS=="ios"', {
                'xcode_settings' : {
                    'SDKROOT': 'iphoneos',
                    'SUPPORTED_PLATFORMS': 'iphonesimulator iphoneos',
                    'ARCHS': 'armv7 armv7s',
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
                    'ONLY_ACTIVE_ARCH': 'YES',
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
