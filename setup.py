from setuptools import setup, find_packages


install_requires = [
    'pyserial',
    'argparse'
]


setup(name="CSLlight",
version="0.0.1",
description="A class to control cameras interfaced with Micro-Manager",
author="Peter Hanappe, Alienor Lahlou",
author_email="peter@hanappe.com",
packages = find_packages(),
install_requires = install_requires,
license="GPLv3",
)