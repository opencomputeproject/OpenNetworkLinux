# 'python setup.py install' to install OOM
# 'python setup.py clean --all install' to clean out old stuff first
from setuptools import setup, find_packages
setup(
    name="oom",
    version="0.5",
    description="Open Optical Monitoring",
    url="https://github.com/ocpnetworking-wip/oom.git",
    author="Don Bollinger",
    author_email="don@thebollingers.org",
    license="MIT",
    packages=find_packages(include=["oom"]),
    package_data={
                  'oom':
                  ['installedshim_parms',
                   'lib/*',
                   'module_data/*',
                   'keyfiles/*']},
    zip_safe=False
)
