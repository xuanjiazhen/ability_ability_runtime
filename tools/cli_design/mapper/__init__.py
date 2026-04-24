"""
CLI Parameter Mapping System

This package provides a flexible system for mapping abstract parameters to command-line arguments.
It supports four mapping types: flag, positional, flattened, and jsonString.
"""

# Import from all_mappers.py (合并后的文件)
from .all_mappers import (
    ArgumentMapper,
    FlagMapper,
    PositionalMapper,
    FlattenedMapper,
    JsonStringMapper,
    MapperFactory,
)

# Import schema validator
from .schema_validator import validate_input, InputSchemaValidator

__version__ = "2.0.0"
__all__ = [
    # Mappers
    "ArgumentMapper",
    "FlagMapper",
    "PositionalMapper",
    "FlattenedMapper",
    "JsonStringMapper",
    "MapperFactory",
    # Schema validation
    "validate_input",
    "InputSchemaValidator",
]
