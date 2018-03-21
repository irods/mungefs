import glob
import itertools
import multiprocessing
import optparse
import os
import sys
import tempfile
import irods_python_ci_utilities

def install_cmake_and_add_to_front_of_path():
    irods_python_ci_utilities.install_os_packages(['irods-externals-cmake3.5.2-0'])
    os.environ['PATH'] = '/opt/irods-externals/cmake3.5.2-0/bin' + os.pathsep + os.environ['PATH']

def get_build_prerequisites_all():
    return ['irods-externals-avro1.7.7-0',
            'irods-externals-boost1.60.0-0',
            'irods-externals-clang3.8-0',
            'irods-externals-cppzmq4.1-0',
            'irods-externals-libarchive3.1.2-0']

def get_build_prerequisites_apt():
    return ['libstdc++6', 'make', 'gcc', 'g++', 'libfuse-dev'] + get_build_prerequisites_all()

def get_build_prerequisites_yum():
    return ['gcc-g++', 'fuse', 'fuse-devel'] + get_build_prerequisites_all()

def get_build_prerequisites():
    dispatch_map = {
        'Ubuntu': get_build_prerequisites_apt,
        'Centos': get_build_prerequisites_yum,
        'Centos linux': get_build_prerequisites_yum,
        'Opensuse ': get_build_prerequisites_yum,
    }
    try:
        return dispatch_map[irods_python_ci_utilities.get_distribution()]()
    except KeyError:
        irods_python_ci_utilities.raise_not_implemented_for_distribution()

def install_build_prerequisites_apt():
    irods_python_ci_utilities.install_os_packages(get_build_prerequisites())

def install_build_prerequisites_yum():
    irods_python_ci_utilities.install_os_packages(get_build_prerequisites())

def install_build_prerequisites():
    dispatch_map = {
        'Ubuntu': install_build_prerequisites_apt,
        'Centos': install_build_prerequisites_yum,
        'Centos linux': install_build_prerequisites_yum,
        'Opensuse ': install_build_prerequisites_yum,
    }
    try:
        return dispatch_map[irods_python_ci_utilities.get_distribution()]()
    except KeyError:
        irods_python_ci_utilities.raise_not_implemented_for_distribution()

def install_irods_dev_and_runtime(irods_build_dir):
    dev_package = 'irods-dev*.{0}'.format(irods_python_ci_utilities.get_package_suffix())
    runtime_package = 'irods-runtime*.{0}'.format(irods_python_ci_utilities.get_package_suffix())
    irods_python_ci_utilities.install_os_packages_from_files(
        itertools.chain(
            glob.glob(os.path.join(irods_build_dir, dev_package)),
            glob.glob(os.path.join(irods_build_dir, runtime_package))))

def build_mungefs(output_root_directory):
    source_directory = os.path.dirname(os.path.realpath(__file__))
    build_directory = tempfile.mkdtemp(prefix='irods_mungefs_build_directory')
    irods_python_ci_utilities.subprocess_get_output(['cmake', source_directory], check_rc=True, cwd=build_directory)
    irods_python_ci_utilities.subprocess_get_output(['make', '-j', str(multiprocessing.cpu_count()), 'package'], check_rc=True, cwd=build_directory)
    if output_root_directory:
        irods_python_ci_utilities.gather_files_satisfying_predicate(
            build_directory,
            irods_python_ci_utilities.append_os_specific_directory(output_root_directory),
            lambda s:s.endswith(irods_python_ci_utilities.get_package_suffix()))

def main():
    # Get option for output directory
    parser = optparse.OptionParser()
    parser.add_option('--output_root_directory')
    parser.add_option('--built_packages_root_directory')
    options,_ = parser.parse_args()

    # Ensure all arguments have been provided
    if not options.output_root_directory:
        print('--output_root_directory must be provided')
        sys.exit(1)

    if not options.built_packages_root_directory:
        print('--built_packages_root_directory must be provided')
        sys.exit(1)

    irods_python_ci_utilities.install_irods_core_dev_repository()
    install_cmake_and_add_to_front_of_path()
    install_build_prerequisites()
    install_irods_dev_and_runtime(
        irods_python_ci_utilities.append_os_specific_directory(options.built_packages_root_directory))
    build_mungefs(options.output_root_directory)

if __name__ == '__main__':
    main()
