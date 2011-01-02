#!/usr/bin/env python

class SmacheException(Exception):
    pass

class ConfigException(SmacheException):
    pass

class BackendException(SmacheException):
    pass
