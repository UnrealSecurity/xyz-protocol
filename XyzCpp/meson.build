project(
    'XyzCpp',
    'c', 'cpp',
    default_options : ['cpp_std=c++17', 'c_std=c11', 'buildtype=debugoptimized'],
    license : 'MIT',
    version : '0.0.5'
)

cxx = meson.get_compiler('cpp')

thread_dep = dependency('threads')
zlib_dep = dependency('zlib')
# it was not being found via normal means on my system
boost_dep = dependency('boost', modules : ['iostreams'])
openssl_dep = dependency('openssl')
ws2_32_dep = []
wsock32_dep = []
mswsock_dep = []
if host_machine.system() == 'windows'
    ws2_32_dep = meson.get_compiler('c').find_library('ws2_32')
    wsock_dep = meson.get_compiler('c').find_library('wsock32')
    mswsock_dep = meson.get_compiler('c').find_library('mswsock')
endif
deps = [
    thread_dep,
    zlib_dep,
    dependency('readline'),
    boost_dep,
    openssl_dep,
    ws2_32_dep,
    wsock32_dep,
    mswsock_dep
]

src = []


#libxyzcpp = shared_library(
#    'xyzcpp',
#    [],
#    include_directories : include_directories('include'),
#    dependencies : deps,
#    install : false
#)

test_client = executable('test_client', sources : ['src/test_client.cpp'], dependencies : deps)
test('test_client', test_client)
test_server = executable('test_server', sources : ['src/test_server.cpp'], dependencies : deps)
test('test_server', test_server)
test_udp_client = executable('test_udp_client', sources : ['src/test_udp_client.cpp'], dependencies : deps)
test('test_udp_client', test_udp_client)
test_udp_server = executable('test_udp_server', sources : ['src/test_udp_server.cpp'], dependencies : deps)
test('test_udp_server', test_udp_server)
libxyzcpp_dep = declare_dependency(
    include_directories : include_directories('include'),
    dependencies : deps
)