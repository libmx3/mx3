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
                ],
            },
        },
        {
            'target_name': 'play',
            'type': 'executable',
            'dependencies': ['libmx3'],
            'sources': [
                'play.cpp',
            ],
        },
    ],
}
