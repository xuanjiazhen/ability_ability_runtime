"""
All-in-One Mapper Package

This module contains all argument mappers and the factory in a single file.
Supports four mapping types: flag, positional, flattened, and jsonString.
"""

from abc import ABC, abstractmethod
from typing import Dict, Any, List
import json


# ============================================================================
# Base Class
# ============================================================================

class ArgumentMapper(ABC):
    """
    Abstract base class for parameter mapping strategies.

    Each mapper implementation converts input parameters according to a specific
    mapping strategy defined in the mapping configuration.
    """

    def __init__(self, mapping_config: Dict[str, Any]):
        """
        Initialize the mapper with a mapping configuration.

        Args:
            mapping_config: Dictionary containing the mapping configuration
        """
        self.mapping_config = mapping_config
        self.templates = mapping_config.get('templates', {})

    @abstractmethod
    def map(self, input_args: Dict[str, Any]) -> List[str]:
        """
        Map input arguments to command-line argument list.

        Args:
            input_args: Dictionary of input parameters

        Returns:
            List of command-line argument strings (argv[1...])
        """
        pass

    def _apply_template(self, template: Any, value: Any) -> str:
        """
        Apply template, replacing {value} placeholder.

        Args:
            template: Template string or dict with if_true/if_false
            value: Value to insert into template

        Returns:
            Result string after template application
        """
        if isinstance(template, str):
            return template.replace('{value}', self._stringify_scalar(value))
        elif isinstance(template, dict):
            if value is True and 'if_true' in template:
                return template['if_true']
            elif value is False and 'if_false' in template:
                return template['if_false']
        return ''

    def _stringify_scalar(self, value: Any) -> str:
        """Convert scalar values to CLI-friendly strings."""
        if isinstance(value, bool):
            return 'true' if value else 'false'
        return str(value)

    def _should_skip_empty(self, result: str) -> bool:
        """
        Check if result should be skipped (empty or whitespace only).

        Args:
            result: String result to check

        Returns:
            True if result should be skipped
        """
        return result == '' or result.isspace()


# ============================================================================
# Flag Mapper
# ============================================================================

class FlagMapper(ArgumentMapper):
    """
    Flag mode mapper: generates --key=value or -v format arguments.

    Processing rules:
    - string: Replace {value} placeholder
    - number/integer: Convert to string, then replace {value}
    - boolean: Supports if_true/if_false templates or {value} string templates
    - array: Iterate through array, generating one argument per element
    - object: Not directly supported (use flattened or jsonString instead)
    """

    def map(self, input_args: Dict[str, Any]) -> List[str]:
        """Map input arguments to flag-style command-line arguments."""
        argv = []

        for field_name, template in self.templates.items():
            if field_name not in input_args:
                continue

            value = input_args[field_name]

            if isinstance(value, bool):
                result = self._apply_template(template, value)
                if not self._should_skip_empty(result):
                    argv.append(result)
            elif isinstance(value, list):
                for item in value:
                    result = self._apply_template(template, item)
                    if not self._should_skip_empty(result):
                        argv.append(result)
            elif isinstance(value, (str, int, float)):
                result = self._apply_template(template, value)
                if not self._should_skip_empty(result):
                    argv.append(result)
            elif isinstance(value, dict):
                raise ValueError(
                    f"Field '{field_name}' is an object, not supported in flag mode. "
                    f"Use 'flattened' or 'jsonString' mode instead."
                )
            else:
                raise ValueError(
                    f"Field '{field_name}' has unsupported type '{type(value).__name__}'"
                )

        return argv


# ============================================================================
# Positional Mapper
# ============================================================================

class PositionalMapper(ArgumentMapper):
    """
    Positional mode mapper: generates arguments in fixed order.

    Processing rules:
    - string/number/integer/boolean: Convert to string and append in order
    - array: Not supported
    - object: Not supported
    """

    def map(self, input_args: Dict[str, Any]) -> List[str]:
        """Map input arguments to positional command-line arguments."""
        argv = []
        order = self.mapping_config.get('order', [])

        for field_name in order:
            if field_name not in input_args:
                continue

            value = input_args[field_name]

            if isinstance(value, list):
                raise ValueError(
                    f"Field '{field_name}' is an array, not supported in positional mode"
                )
            if isinstance(value, dict):
                raise ValueError(
                    f"Field '{field_name}' is an object, not supported in positional mode"
                )

            if isinstance(value, bool):
                argv.append('true' if value else 'false')
            else:
                argv.append(str(value))

        return argv


# ============================================================================
# Flattened Mapper
# ============================================================================

class FlattenedMapper(ArgumentMapper):
    """
    Flattened mode mapper: expands nested objects into path-based parameters.

    Processing rules:
    - string/number/integer: Replace {value} placeholder
    - boolean: Supports if_true/if_false templates or {value} string templates
    - array: Not supported at leaf level
    - object: Recursively flatten using separator
    """

    def __init__(self, mapping_config: Dict[str, Any]):
        """Initialize the flattened mapper."""
        super().__init__(mapping_config)
        self.separator = mapping_config.get('separator', '.')

    def map(self, input_args: Dict[str, Any]) -> List[str]:
        """Map input arguments to flattened command-line arguments."""
        argv = []
        flattened = self._flatten_object(input_args)

        for field_name, template in self.templates.items():
            if field_name not in flattened:
                continue

            value = flattened[field_name]

            if isinstance(value, bool):
                result = self._apply_template(template, value)
                if not self._should_skip_empty(result):
                    argv.append(result)
            elif isinstance(value, list):
                raise ValueError(
                    f"Field '{field_name}' is an array, not supported in flattened mode"
                )
            elif isinstance(value, (str, int, float)):
                result = self._apply_template(template, value)
                if not self._should_skip_empty(result):
                    argv.append(result)
            else:
                raise ValueError(
                    f"Field '{field_name}' has unsupported type '{type(value).__name__}'"
                )

        return argv

    def _flatten_object(self, obj: Dict[str, Any], prefix: str = '') -> Dict[str, Any]:
        """Recursively flatten nested objects."""
        result = {}

        for key, value in obj.items():
            new_key = f"{prefix}{self.separator}{key}" if prefix else key

            if isinstance(value, dict):
                result.update(self._flatten_object(value, new_key))
            else:
                result[new_key] = value

        return result


# ============================================================================
# Json-String Mapper
# ============================================================================

class JsonStringMapper(ArgumentMapper):
    """
    JSON-string mode mapper: serializes complex structures to JSON strings.

    Processing rules:
    - object/array: Serialize to JSON string, then replace {value} placeholder
    - boolean: Supports if_true/if_false templates or {value} string templates
    - string/number/integer: Replace {value} with scalar string
    """

    def map(self, input_args: Dict[str, Any]) -> List[str]:
        """Map input arguments to JSON-string command-line arguments."""
        argv = []

        for field_name, template in self.templates.items():
            if field_name not in input_args:
                continue

            value = input_args[field_name]
            if isinstance(template, dict):
                result = self._apply_template(template, value)
            else:
                if isinstance(value, (dict, list)):
                    formatted = json.dumps(value, ensure_ascii=False)
                else:
                    formatted = self._stringify_scalar(value)
                result = template.replace('{value}', formatted)
            argv.append(result)

        return argv


# ============================================================================
# Mixed Mapper
# ============================================================================

class MixedMapper(ArgumentMapper):
    """
    Mixed mode mapper: 组合使用 flag、positional、flattened、jsonString 模式

    配置结构：
    {
        "type": "mixed",
        "mappings": {
            "<param>": {
                "mode": "flag|positional|flattened|jsonString",
                "template": ...,           # flag/jsonString/flattened 需要
                "separator": ".",          # flattened 可选
                "order": 1                 # positional 必需
            }
        }
    }

    输出顺序：
    1. 所有 mode=position 的条目按 order 升序输出
    2. 其余条目按 mappings 定义顺序输出
    """

    def __init__(self, mapping_config: Dict[str, Any]):
        """
        初始化 mixed 映射器

        Args:
            mapping_config: mixed 映射配置
        """
        super().__init__(mapping_config)
        self.mappings = mapping_config.get('mappings', {})

        if not self.mappings:
            raise ValueError("mixed mode requires 'mappings' field")

    def map(self, input_args: Dict[str, Any]) -> List[str]:
        """
        映射输入参数为命令行参数

        Args:
            input_args: 输入参数字典

        Returns:
            命令行参数列表
        """
        argv = []

        # 第一步：收集所有 positional 条目，按 order 排序
        positional_entries = []
        non_positional_entries = []

        for param_name, param_config in self.mappings.items():
            mode = param_config.get('mode')
            if not mode:
                raise ValueError(f"Missing 'mode' for parameter '{param_name}'")

            if mode == 'positional':
                order = param_config.get('order')
                if not order or not isinstance(order, int) or order <= 0:
                    raise ValueError(
                        f"positional mode requires positive integer 'order' for '{param_name}'"
                    )
                # 检查 order 重复
                for existing_name, existing_order in [(p['name'], p['order']) for p in positional_entries]:
                    if existing_order == order:
                        raise ValueError(
                            f"Duplicate order '{order}' for parameters '{existing_name}' and '{param_name}'"
                        )
                positional_entries.append({
                    'name': param_name,
                    'order': order,
                    'config': param_config
                })
            else:
                non_positional_entries.append({
                    'name': param_name,
                    'config': param_config,
                    'index': len(non_positional_entries)  # 保存原始顺序
                })

        # 第二步：按 order 升序处理 positional
        positional_entries.sort(key=lambda x: x['order'])
        for entry in positional_entries:
            param_name = entry['name']
            param_config = entry['config']
            # 创建临时的 positional 映射配置
            temp_config = {
                'type': 'positional',
                'order': [param_name]
            }
            sub_mapper = PositionalMapper(temp_config)
            # 只传入当前字段
            if param_name in input_args:
                param_value = {param_name: input_args[param_name]}
                argv.extend(sub_mapper.map(param_value))

        # 第三步：按定义顺序处理非 positional
        non_positional_entries.sort(key=lambda x: x['index'])
        for entry in non_positional_entries:
            param_name = entry['name']
            param_config = entry['config']
            mode = param_config['mode']

            # 根据模式创建子映射器
            if mode == 'flag':
                sub_mapper = self._create_flag_mapper(param_name, param_config)
            elif mode == 'flattened':
                sub_mapper = self._create_flattened_mapper(param_name, param_config)
            elif mode == 'jsonString':
                sub_mapper = self._create_json_string_mapper(param_name, param_config)
            else:
                raise ValueError(f"Unsupported mode '{mode}' for parameter '{param_name}'")

            # 提取相关数据并映射
            if mode in ['flag', 'jsonString']:
                # flag 和 jsonString 直接引用字段
                if param_name in input_args:
                    param_value = {param_name: input_args[param_name]}
                    argv.extend(sub_mapper.map(param_value))
            elif mode == 'flattened':
                # flattened 需要传入整个输入，因为模板会提取路径
                argv.extend(sub_mapper.map(input_args))

        return argv

    def _create_flag_mapper(self, param_name: str, param_config: Dict[str, Any]) -> 'FlagMapper':
        """创建 flag 子映射器"""
        template = param_config.get('template')
        if not template:
            raise ValueError(f"flag mode requires 'template' for '{param_name}'")

        temp_config = {
            'type': 'flag',
            'templates': {
                param_name: template
            }
        }
        return FlagMapper(temp_config)

    def _create_flattened_mapper(self, param_name: str, param_config: Dict[str, Any]) -> 'FlattenedMapper':
        """创建 flattened 子映射器"""
        template = param_config.get('template')
        if not template or not isinstance(template, dict):
            raise ValueError(f"flattened mode requires non-empty object 'template' for '{param_name}'")

        separator = param_config.get('separator', '.')

        # 验证所有路径 key 都以 param_name 为根
        for path_key in template.keys():
            if not path_key.startswith(f"{param_name}{separator}"):
                raise ValueError(
                    f"flattened template path '{path_key}' must start with '{param_name}{separator}'"
                )

        temp_config = {
            'type': 'flattened',
            'separator': separator,
            'templates': template
        }
        return FlattenedMapper(temp_config)

    def _create_json_string_mapper(self, param_name: str, param_config: Dict[str, Any]) -> 'JsonStringMapper':
        """创建 jsonString 子映射器"""
        template = param_config.get('template')
        if not template:
            raise ValueError(f"jsonString mode requires 'template' for '{param_name}'")

        if not isinstance(template, str):
            raise ValueError(f"jsonString template must be a string for '{param_name}'")

        temp_config = {
            'type': 'jsonString',
            'templates': {
                param_name: template
            }
        }
        return JsonStringMapper(temp_config)


# ============================================================================
# Mapper Factory
# ============================================================================

class MapperFactory:
    """
    Factory for creating argument mappers.

    Supported mapping types:
    - flag: Standard --key=value or -v format
    - positional: Fixed-order positional arguments
    - flattened: Path-based expansion of nested objects
    - jsonString: JSON serialization of complex structures
    - mixed: Combination of multiple mapping modes
    """

    # Registry of available mappers
    _MAPPERS = {
        'flag': FlagMapper,
        'positional': PositionalMapper,
        'flattened': FlattenedMapper,
        'jsonString': JsonStringMapper,
        'mixed': MixedMapper,
    }

    @staticmethod
    def create(mapping_config: Dict[str, Any]) -> ArgumentMapper:
        """
        Create an argument mapper based on mapping type.

        Args:
            mapping_config: Dictionary containing the mapping configuration

        Returns:
            ArgumentMapper instance

        Raises:
            ValueError: If mapping type is unsupported
        """
        mapping_type = mapping_config.get('type')

        if not mapping_type:
            raise ValueError("Mapping configuration must specify 'type' field")

        if mapping_type not in MapperFactory._MAPPERS:
            supported_types = list(MapperFactory._MAPPERS.keys())
            raise ValueError(
                f"Unsupported mapping type: '{mapping_type}'. "
                f"Supported types: {supported_types}"
            )

        mapper_class = MapperFactory._MAPPERS[mapping_type]
        return mapper_class(mapping_config)

    @staticmethod
    def get_supported_types() -> list:
        """Get list of supported mapping types."""
        return list(MapperFactory._MAPPERS.keys())

    @staticmethod
    def is_type_supported(mapping_type: str) -> bool:
        """Check if a mapping type is supported."""
        return mapping_type in MapperFactory._MAPPERS


# Export all classes
__all__ = [
    "ArgumentMapper",
    "FlagMapper",
    "PositionalMapper",
    "FlattenedMapper",
    "JsonStringMapper",
    "MixedMapper",
    "MapperFactory",
]
