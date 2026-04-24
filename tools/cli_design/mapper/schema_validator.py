"""
Input 验证器 - 基于 JSON Schema 子集的参数校验

根据当前 CliTool 规范中定义的 JSON Schema 受限子集实现参数验证。
"""

import re
from typing import Any, Dict, List, Optional, Tuple
from datetime import datetime


class SchemaValidationError(Exception):
    """Schema 验证错误"""

    def __init__(self, field_path: str, message: str):
        self.field_path = field_path
        self.message = message
        super().__init__(f"[{field_path}] {message}")


class InputSchemaValidator:
    """Input 验证器"""

    # 支持的 format 正则表达式
    FORMAT_PATTERNS = {
        'ipv4': r'^^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$',
        'ipv6': r'^^([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}$',
        'hostname': r'^^(?:[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?\.)*[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?$',
        'email': r'^^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$',
        'uri': r'^^[a-zA-Z][a-zA-Z0-9+.-]*:[^\\s]*$',
    }

    def __init__(self, schema: Dict[str, Any]):
        """
        初始化验证器

        Args:
            schema: input 定义
        """
        self.schema = schema

    def validate(self, input_args: Dict[str, Any]) -> Tuple[bool, List[str]]:
        """
        验证输入参数

        Args:
            input_args: 输入参数对象

        Returns:
            (是否通过, 错误列表)
        """
        errors = []

        # 验证基本结构
        if self.schema.get('type') != 'object':
            errors.append("Schema root type must be 'object'")
            return False, errors

        # 获取 properties 和 required
        properties = self.schema.get('properties', {})
        required = set(self.schema.get('required', []))

        # 验证必填字段
        for field in required:
            if field not in input_args:
                errors.append(f"Missing required field: {field}")

        # 验证每个字段
        for field_name, field_value in input_args.items():
            if field_name not in properties:
                errors.append(f"Input field is not declared in input.properties: {field_name}")
                continue

            field_schema = properties[field_name]

            try:
                self._validate_field(field_name, field_value, field_schema)
            except SchemaValidationError as e:
                errors.append(str(e))

        return len(errors) == 0, errors

    def _validate_field(self, field_path: str, value: Any, schema: Dict[str, Any]) -> None:
        """
        验证单个字段

        Args:
            field_path: 字段路径（用于错误信息）
            value: 字段值
            schema: 字段 schema 定义

        Raises:
            SchemaValidationError: 验证失败
        """
        expected_type = schema.get('type')

        if expected_type is None:
            raise SchemaValidationError(field_path, "Missing type in schema")

        # 类型验证
        if expected_type == 'string':
            self._validate_string(field_path, value, schema)
        elif expected_type == 'number':
            self._validate_number(field_path, value, schema)
        elif expected_type == 'integer':
            self._validate_integer(field_path, value, schema)
        elif expected_type == 'boolean':
            self._validate_boolean(field_path, value)
        elif expected_type == 'array':
            self._validate_array(field_path, value, schema)
        elif expected_type == 'object':
            self._validate_object(field_path, value, schema)
        else:
            raise SchemaValidationError(field_path, f"Unsupported type: {expected_type}")

    def _validate_string(self, field_path: str, value: Any, schema: Dict[str, Any]) -> None:
        """验证字符串类型"""
        if not isinstance(value, str):
            raise SchemaValidationError(field_path,
                                       f"Expected string, got {type(value).__name__}")

        # minLength
        min_length = schema.get('minLength')
        if min_length is not None and len(value) < min_length:
            raise SchemaValidationError(field_path,
                                       f"Length {len(value)} < minLength {min_length}")

        # maxLength
        max_length = schema.get('maxLength')
        if max_length is not None and len(value) > max_length:
            raise SchemaValidationError(field_path,
                                       f"Length {len(value)} > maxLength {max_length}")

        # pattern
        pattern = schema.get('pattern')
        if pattern is not None:
            if not re.match(pattern, value):
                raise SchemaValidationError(field_path,
                                           f"Value does not match pattern: {pattern}")

        # format
        format_type = schema.get('format')
        if format_type is not None:
            self._validate_format(field_path, value, format_type)

    def _validate_format(self, field_path: str, value: str, format_type: str) -> None:
        """验证格式"""
        if format_type in ['date', 'date-time']:
            self._validate_date_format(field_path, value, format_type)
        elif format_type in self.FORMAT_PATTERNS:
            pattern = self.FORMAT_PATTERNS[format_type]
            if not re.match(pattern, value):
                raise SchemaValidationError(field_path,
                                           f"Value does not match format: {format_type}")
        else:
            # 未知格式，跳过验证
            pass

    def _validate_date_format(self, field_path: str, value: str, format_type: str) -> None:
        """验证日期格式"""
        try:
            if format_type == 'date':
                # RFC 3339 full-date: YYYY-MM-DD
                datetime.strptime(value, '%Y-%m-%d')
            elif format_type == 'date-time':
                # RFC 3339 date-time
                # 支持多种时区格式
                formats = [
                    '%Y-%m-%dT%H:%M:%SZ',           # 2023-01-01T00:00:00Z
                    '%Y-%m-%dT%H:%M:%S.%fZ',        # 2023-01-01T00:00:00.000Z
                    '%Y-%m-%dT%H:%M:%S%z',          # 2023-01-01T00:00:00+08:00
                    '%Y-%m-%dT%H:%M:%S.%f%z',       # 2023-01-01T00:00:00.000+08:00
                ]
                parsed = False
                for fmt in formats:
                    try:
                        datetime.strptime(value, fmt)
                        parsed = True
                        break
                    except ValueError:
                        continue

                if not parsed:
                    raise ValueError("No date-time format matched")
        except ValueError as e:
            raise SchemaValidationError(field_path,
                                       f"Invalid {format_type} format: {value}")

    def _validate_number(self, field_path: str, value: Any, schema: Dict[str, Any]) -> None:
        """验证数字类型"""
        if not isinstance(value, (int, float)):
            raise SchemaValidationError(field_path,
                                       f"Expected number, got {type(value).__name__}")

        # 排除整数
        if isinstance(value, bool):
            raise SchemaValidationError(field_path, "Boolean is not a number")

        if isinstance(value, int) and not isinstance(value, bool):
            # 整数也符合 number 类型
            pass

        # minimum
        minimum = schema.get('minimum')
        if minimum is not None and value < minimum:
            raise SchemaValidationError(field_path,
                                       f"Value {value} < minimum {minimum}")

        # maximum
        maximum = schema.get('maximum')
        if maximum is not None and value > maximum:
            raise SchemaValidationError(field_path,
                                       f"Value {value} > maximum {maximum}")

        # exclusiveMinimum
        exclusive_minimum = schema.get('exclusiveMinimum')
        if exclusive_minimum is not None and value <= exclusive_minimum:
            raise SchemaValidationError(field_path,
                                       f"Value {value} <= exclusiveMinimum {exclusive_minimum}")

        # exclusiveMaximum
        exclusive_maximum = schema.get('exclusiveMaximum')
        if exclusive_maximum is not None and value >= exclusive_maximum:
            raise SchemaValidationError(field_path,
                                       f"Value {value} >= exclusiveMaximum {exclusive_maximum}")

        # multipleOf
        multiple_of = schema.get('multipleOf')
        if multiple_of is not None and multiple_of != 0:
            if not float(value).is_integer() and not float(value / multiple_of).is_integer():
                # 对于整数，直接判断能否整除
                # 对于浮点数，检查是否为倍数
                if value % multiple_of != 0:
                    raise SchemaValidationError(field_path,
                                               f"Value {value} is not a multiple of {multiple_of}")

    def _validate_integer(self, field_path: str, value: Any, schema: Dict[str, Any]) -> None:
        """验证整数类型"""
        if not isinstance(value, int) or isinstance(value, bool):
            raise SchemaValidationError(field_path,
                                       f"Expected integer, got {type(value).__name__}")

        # 复用 number 的验证逻辑
        self._validate_number(field_path, value, schema)

    def _validate_boolean(self, field_path: str, value: Any) -> None:
        """验证布尔类型"""
        if not isinstance(value, bool):
            raise SchemaValidationError(field_path,
                                       f"Expected boolean, got {type(value).__name__}")

    def _validate_array(self, field_path: str, value: Any, schema: Dict[str, Any]) -> None:
        """验证数组类型"""
        if not isinstance(value, list):
            raise SchemaValidationError(field_path,
                                       f"Expected array, got {type(value).__name__}")

        # minItems
        min_items = schema.get('minItems')
        if min_items is not None and len(value) < min_items:
            raise SchemaValidationError(field_path,
                                       f"Array length {len(value)} < minItems {min_items}")

        # maxItems
        max_items = schema.get('maxItems')
        if max_items is not None and len(value) > max_items:
            raise SchemaValidationError(field_path,
                                       f"Array length {len(value)} > maxItems {max_items}")

        # uniqueItems
        unique_items = schema.get('uniqueItems')
        if unique_items and len(value) != len(set(str(v) for v in value)):
            raise SchemaValidationError(field_path, "Array items are not unique")

        # items 验证
        items_schema = schema.get('items')
        if items_schema is not None:
            for i, item in enumerate(value):
                try:
                    self._validate_field(f"{field_path}[{i}]", item, items_schema)
                except SchemaValidationError as e:
                    raise SchemaValidationError(e.field_path, e.message)

    def _validate_object(self, field_path: str, value: Any, schema: Dict[str, Any]) -> None:
        """验证对象类型"""
        if not isinstance(value, dict):
            raise SchemaValidationError(field_path,
                                       f"Expected object, got {type(value).__name__}")

        # properties 验证
        properties = schema.get('properties', {})
        required = set(schema.get('required', []))

        # 验证必填字段
        for field in required:
            if field not in value:
                raise SchemaValidationError(field_path,
                                           f"Missing required property: {field}")

        # 验证每个属性
        for prop_name, prop_value in value.items():
            if prop_name not in properties:
                raise SchemaValidationError(
                    field_path,
                    f"Input field is not declared in input.properties: {prop_name}"
                )

            prop_schema = properties[prop_name]
            try:
                self._validate_field(f"{field_path}.{prop_name}", prop_value, prop_schema)
            except SchemaValidationError as e:
                raise SchemaValidationError(e.field_path, e.message)


# 便捷函数
def validate_input(input_args: Dict[str, Any], schema: Dict[str, Any]) -> Tuple[bool, List[str]]:
    """
    验证输入参数

    Args:
        input_args: 输入参数
        schema: input 定义

    Returns:
        (是否通过, 错误列表)
    """
    validator = InputSchemaValidator(schema)
    return validator.validate(input_args)


if __name__ == '__main__':
    # 测试示例
    test_schema = {
        "type": "object",
        "properties": {
            "target": {
                "type": "string",
                "format": "ipv4",
                "minLength": 7,
                "maxLength": 15
            },
            "ports": {
                "type": "array",
                "items": {"type": "integer", "minimum": 1, "maximum": 65535},
                "minItems": 1,
                "uniqueItems": True
            },
            "timeout": {
                "type": "integer",
                "minimum": 1,
                "maximum": 300,
                "default": 5
            },
            "verbose": {
                "type": "boolean",
                "default": False
            }
        },
        "required": ["target"]
    }

    # 测试用例
    test_cases = [
        # 正确的输入
        ({
            "target": "192.168.1.1",
            "ports": [80, 443],
            "timeout": 30,
            "verbose": True
        }, True, "Valid input"),

        # 缺少必填字段
        ({
            "ports": [80],
            "timeout": 30
        }, False, "Missing required field 'target'"),

        # 类型错误
        ({
            "target": "192.168.1.1",
            "timeout": "30"  # 应该是 integer
        }, False, "timeout should be integer"),

        # 格式错误
        ({
            "target": "999.999.999.999",  # 无效的 IP
            "ports": [80]
        }, False, "Invalid IPv4 format"),

        # 约束违反
        ({
            "target": "192.168.1.1",
            "ports": [0]  # 端口 < 1
        }, False, "Port 0 < minimum 1"),
    ]

    print("Input 验证测试")
    print("=" * 60)

    for input_args, expected_valid, description in test_cases:
        is_valid, errors = validate_input(input_args, test_schema)

        status = "✅ PASS" if is_valid == expected_valid else "❌ FAIL"
        print(f"\n{status}: {description}")
        print(f"输入: {input_args}")

        if not is_valid:
            print("错误:")
            for error in errors:
                print(f"  - {error}")
