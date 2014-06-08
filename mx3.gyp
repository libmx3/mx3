{
    'targets': [
        {
            'target_name': 'libmx3',
            'type': 'static_library',
            'conditions': [],
            'dependencies': [
                'deps/leveldb.gyp:leveldb',
                'deps/json11.gyp:json11',
                'deps/sqlite3.gyp:sqlite3',
                'deps/CppSQLite.gyp:CppSQLite',
            ],
            'sources': [
                # just automatically include all cpp and hpp files in src/ (for now)
                # '<!' is shell expand
                # '@' is to splat the arguments into list items
                # todo(kabbes) this will not work on windows
                '<!@(find src -name "*.cpp" -o -name "*.hpp")',
            ],
            'include_dirs': [
                'include',
            ],
            'all_dependent_settings': {
                'include_dirs': [
                    'include',
                    'deps',
                ],
            },
        },
        {
            'target_name': 'libmx3_objc',
            'type': 'static_library',
            'conditions': [],
            'dependencies': [ 'libmx3' ],
            'sources': [
                '<!@(find objc -name "*.mm" -o -name "*.h" -o -name "*.m")',
            ],
            'sources!': ['play.m'],
            'include_dirs': [
                'include',
                'objc',
            ],
            'all_dependent_settings': {
                'include_dirs': [
                    'include',
                    'objc',
                ],
            },
        },
        {
            'target_name': 'mx3_jni',
            'type': 'shared_library',
            'conditions': [],
            'dependencies': [ 'libmx3' ],
            'sources': [
                '<!@(find android -name "*.cpp" -o -name "*.c")',
            ],
        },
        {
            'target_name': 'play_objc',
            'type': 'executable',
            'dependencies': ['libmx3_objc'],
            # I'm not sure why you have to specify libc++ when you build this :(
            'libraries': [
                'libc++.a',
            ],
            'sources': [
                'objc/play.m',
            ],
        },
        {
            'target_name': 'test',
            'type': 'executable',
            'dependencies': [
                'libmx3',
                'deps/gtest.gyp:gtest',
            ],
            'include_dirs': [
                '.',
                'test',
            ],
            'sources': [
                '<!@(find test -name "*.cpp" -o -name "*.hpp")',
            ]
        },
    ],
}
