"""
All-in-One Test Suite

合并后的测试文件，包含所有测试用例：
- 代码映射测试 (test_mappers.py)
- 文件映射测试 (test_file_mappers.py)
- Schema 验证测试 (test_schema_validation.py)
"""

import unittest
import json
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent.parent))

from mapper import MapperFactory
from mapper.schema_validator import validate_input


# ============================================================================
# 代码映射测试 (原 test_mappers.py)
# ============================================================================

class TestFlagMapper(unittest.TestCase):
    """Flag模式测试"""

    def test_basic_flag_mapping(self):
        """测试基本标志映射"""
        config = {
            'type': 'flag',
            'templates': {
                'target': '--target={value}',
                'timeout': '--timeout={value}',
                'verbose': {'if_true': '-v', 'if_false': ''}
            }
        }
        mapper = MapperFactory.create(config)
        input_args = {'target': '192.168.1.1', 'timeout': 5, 'verbose': True}
        result = mapper.map(input_args)
        self.assertEqual(result, ['--target=192.168.1.1', '--timeout=5', '-v'])

    def test_array_expansion(self):
        """测试数组展开"""
        config = {'type': 'flag', 'templates': {'ports': '--port={value}'}}
        mapper = MapperFactory.create(config)
        input_args = {'ports': [80, 443, 8080]}
        result = mapper.map(input_args)
        self.assertEqual(result, ['--port=80', '--port=443', '--port=8080'])

    def test_boolean_template(self):
        """测试布尔值模板"""
        config = {
            'type': 'flag',
            'templates': {'verbose': {'if_true': '-v', 'if_false': '--quiet'}}
        }
        mapper = MapperFactory.create(config)
        result_true = mapper.map({'verbose': True})
        result_false = mapper.map({'verbose': False})
        self.assertEqual(result_true, ['-v'])
        self.assertEqual(result_false, ['--quiet'])

    def test_boolean_string_template(self):
        """测试布尔值字符串模板"""
        config = {
            'type': 'flag',
            'templates': {'verbose': '--verbose={value}'}
        }
        mapper = MapperFactory.create(config)
        self.assertEqual(mapper.map({'verbose': True}), ['--verbose=true'])
        self.assertEqual(mapper.map({'verbose': False}), ['--verbose=false'])

    def test_skip_missing_fields(self):
        """测试跳过缺失字段"""
        config = {
            'type': 'flag',
            'templates': {'target': '--target={value}', 'optional': '--optional={value}'}
        }
        mapper = MapperFactory.create(config)
        result = mapper.map({'target': '192.168.1.1'})
        self.assertEqual(result, ['--target=192.168.1.1'])


class TestPositionalMapper(unittest.TestCase):
    """Positional模式测试"""

    def test_basic_positional_mapping(self):
        """测试基本位置参数映射"""
        config = {'type': 'positional', 'order': ['source', 'dest']}
        mapper = MapperFactory.create(config)
        input_args = {'source': '/data/file.txt', 'dest': '/backup/file.txt'}
        result = mapper.map(input_args)
        self.assertEqual(result, ['/data/file.txt', '/backup/file.txt'])

    def test_boolean_conversion(self):
        """测试布尔值转换"""
        config = {'type': 'positional', 'order': ['source', 'dest', 'verbose']}
        mapper = MapperFactory.create(config)
        input_args = {'source': '/data/file.txt', 'dest': '/backup/file.txt', 'verbose': True}
        result = mapper.map(input_args)
        self.assertEqual(result, ['/data/file.txt', '/backup/file.txt', 'true'])

    def test_number_conversion(self):
        """测试数字转换"""
        config = {'type': 'positional', 'order': ['count', 'size']}
        mapper = MapperFactory.create(config)
        input_args = {'count': 5, 'size': 1024}
        result = mapper.map(input_args)
        self.assertEqual(result, ['5', '1024'])


class TestFlattenedMapper(unittest.TestCase):
    """Flattened模式测试"""

    def test_basic_flattening(self):
        """测试基本对象展平"""
        config = {
            'type': 'flattened',
            'separator': '.',
            'templates': {
                'app.name': '--app-name={value}',
                'server.host': '--host={value}'
            }
        }
        mapper = MapperFactory.create(config)
        input_args = {'app': {'name': 'myapp'}, 'server': {'host': '192.168.1.100'}}
        result = mapper.map(input_args)
        self.assertEqual(result, ['--app-name=myapp', '--host=192.168.1.100'])

    def test_nested_flattening(self):
        """测试嵌套对象递归展平"""
        config = {
            'type': 'flattened',
            'separator': '.',
            'templates': {
                'app.name': '--app-name={value}',
                'app.env.production': {'if_true': '--env-prod', 'if_false': ''}
            }
        }
        mapper = MapperFactory.create(config)
        input_args = {'app': {'name': 'myapp', 'env': {'production': True}}}
        result = mapper.map(input_args)
        self.assertEqual(result, ['--app-name=myapp', '--env-prod'])

    def test_boolean_string_template(self):
        """测试展平模式下布尔字符串模板"""
        config = {
            'type': 'flattened',
            'separator': '.',
            'templates': {
                'app.env.production': '--env-production={value}'
            }
        }
        mapper = MapperFactory.create(config)
        input_args = {'app': {'env': {'production': True}}}
        result = mapper.map(input_args)
        self.assertEqual(result, ['--env-production=true'])


class TestJsonStringMapper(unittest.TestCase):
    """Json-String模式测试"""

    def test_basic_json_serialization(self):
        """测试基本对象JSON序列化"""
        config = {'type': 'jsonString', 'templates': {'config': '--config={value}'}}
        mapper = MapperFactory.create(config)
        input_args = {'config': {'application': {'name': 'webapp', 'replicas': 3}}}
        result = mapper.map(input_args)
        self.assertEqual(len(result), 1)
        self.assertTrue(result[0].startswith('--config='))
        json_str = result[0].replace('--config=', '')
        parsed = json.loads(json_str)
        self.assertEqual(parsed['application']['name'], 'webapp')

    def test_unicode_preservation(self):
        """测试Unicode字符保持"""
        config = {'type': 'jsonString', 'templates': {'message': '--message={value}'}}
        mapper = MapperFactory.create(config)
        input_args = {'message': '你好世界'}
        result = mapper.map(input_args)
        self.assertTrue('你好世界' in result[0])

    def test_boolean_template(self):
        """测试 jsonString 模式布尔模板"""
        config = {'type': 'jsonString', 'templates': {'enabled': '--enabled={value}'}}
        mapper = MapperFactory.create(config)
        self.assertEqual(mapper.map({'enabled': True}), ['--enabled=true'])


class TestMixedMapper(unittest.TestCase):
    """Mixed模式测试"""

    def test_basic_mixed_mapping(self):
        """测试基本混合映射"""
        config = {
            'type': 'mixed',
            'mappings': {
                'action': {'mode': 'positional', 'order': 1},
                'verbose': {'mode': 'flag', 'template': {'if_true': '-v', 'if_false': ''}}
            }
        }
        mapper = MapperFactory.create(config)
        input_args = {'action': 'deploy', 'verbose': True}
        result = mapper.map(input_args)
        self.assertEqual(result, ['deploy', '-v'])

    def test_positional_order(self):
        """测试位置参数顺序"""
        config = {
            'type': 'mixed',
            'mappings': {
                'service': {'mode': 'positional', 'order': 2},
                'action': {'mode': 'positional', 'order': 1}
            }
        }
        mapper = MapperFactory.create(config)
        input_args = {'action': 'deploy', 'service': 'webapp'}
        result = mapper.map(input_args)
        self.assertEqual(result, ['deploy', 'webapp'])

    def test_mixed_with_flattened(self):
        """测试混合展平模式"""
        config = {
            'type': 'mixed',
            'mappings': {
                'action': {'mode': 'positional', 'order': 1},
                'server': {
                    'mode': 'flattened',
                    'template': {'server.host': '--host={value}'},
                    'separator': '.'
                }
            }
        }
        mapper = MapperFactory.create(config)
        input_args = {'action': 'deploy', 'server': {'host': '192.168.1.1'}}
        result = mapper.map(input_args)
        self.assertEqual(result, ['deploy', '--host=192.168.1.1'])

    def test_mixed_with_json_string(self):
        """测试混合JSON字符串模式"""
        config = {
            'type': 'mixed',
            'mappings': {
                'action': {'mode': 'positional', 'order': 1},
                'config': {'mode': 'jsonString', 'template': '--config={value}'}
            }
        }
        mapper = MapperFactory.create(config)
        input_args = {'action': 'deploy', 'config': {'replicas': 3}}
        result = mapper.map(input_args)
        self.assertEqual(len(result), 2)
        self.assertEqual(result[0], 'deploy')
        self.assertTrue(result[1].startswith('--config='))

    def test_complex_mixed_mapping(self):
        """测试复杂混合映射"""
        config = {
            'type': 'mixed',
            'mappings': {
                'action': {'mode': 'positional', 'order': 1},
                'service': {'mode': 'positional', 'order': 2},
                'verbose': {'mode': 'flag', 'template': {'if_true': '--verbose', 'if_false': ''}},
                'dryRun': {'mode': 'flag', 'template': {'if_true': '--dry-run', 'if_false': ''}},
                'server': {
                    'mode': 'flattened',
                    'template': {
                        'server.host': '--host={value}',
                        'server.port': '--port={value}'
                    },
                    'separator': '.'
                },
                'config': {'mode': 'jsonString', 'template': '--config={value}'}
            }
        }
        mapper = MapperFactory.create(config)
        input_args = {
            'action': 'deploy',
            'service': 'gateway',
            'verbose': True,
            'dryRun': False,
            'server': {'host': 'api.example.com', 'port': 443},
            'config': {'cpu': '2'}
        }
        result = mapper.map(input_args)
        # positional 先按 order 输出
        self.assertEqual(result[0], 'deploy')
        self.assertEqual(result[1], 'gateway')
        # flag 按定义顺序
        self.assertEqual(result[2], '--verbose')
        # flattened 按定义顺序
        self.assertTrue('--host=api.example.com' in result)
        self.assertTrue('--port=443' in result)
        # jsonString 最后
        self.assertTrue(any(r.startswith('--config=') for r in result))

    def test_missing_mappings_field(self):
        """测试缺少 mappings 字段"""
        config = {'type': 'mixed'}
        with self.assertRaises(ValueError) as context:
            MapperFactory.create(config)
        self.assertIn('mappings', str(context.exception))

    def test_invalid_mode(self):
        """测试非法模式"""
        config = {
            'type': 'mixed',
            'mappings': {
                'action': {'mode': 'invalid'}
            }
        }
        mapper = MapperFactory.create(config)
        with self.assertRaises(ValueError):
            mapper.map({'action': 'deploy'})


# ============================================================================
# Schema 验证测试 (原 test_schema_validation.py)
# ============================================================================

class TestInputValidation(unittest.TestCase):
    """Input 验证测试"""

    def test_valid_input(self):
        """测试有效输入"""
        schema = {
            "type": "object",
            "properties": {
                "target": {"type": "string", "format": "ipv4"},
                "ports": {"type": "array", "items": {"type": "integer"}}
            },
            "required": ["target"]
        }
        input_args = {"target": "192.168.1.1", "ports": [80, 443]}
        is_valid, errors = validate_input(input_args, schema)
        self.assertTrue(is_valid)
        self.assertEqual(len(errors), 0)

    def test_missing_required_field(self):
        """测试缺少必填字段"""
        schema = {
            "type": "object",
            "properties": {"target": {"type": "string"}},
            "required": ["target"]
        }
        input_args = {"ports": [80]}
        is_valid, errors = validate_input(input_args, schema)
        self.assertFalse(is_valid)
        self.assertTrue(any("target" in error for error in errors))

    def test_type_mismatch(self):
        """测试类型不匹配"""
        schema = {
            "type": "object",
            "properties": {"timeout": {"type": "integer"}}
        }
        input_args = {"timeout": "30"}
        is_valid, errors = validate_input(input_args, schema)
        self.assertFalse(is_valid)

    def test_constraint_violation(self):
        """测试约束违反"""
        schema = {
            "type": "object",
            "properties": {"port": {"type": "integer", "minimum": 1, "maximum": 65535}}
        }
        input_args = {"port": 0}
        is_valid, errors = validate_input(input_args, schema)
        self.assertFalse(is_valid)


# ============================================================================
# 运行所有测试
# ============================================================================

def run_all_tests():
    """运行所有测试"""
    suite = unittest.TestSuite()

    # 添加所有测试类
    suite.addTests(unittest.TestLoader().loadTestsFromTestCase(TestFlagMapper))
    suite.addTests(unittest.TestLoader().loadTestsFromTestCase(TestPositionalMapper))
    suite.addTests(unittest.TestLoader().loadTestsFromTestCase(TestFlattenedMapper))
    suite.addTests(unittest.TestLoader().loadTestsFromTestCase(TestJsonStringMapper))
    suite.addTests(unittest.TestLoader().loadTestsFromTestCase(TestMixedMapper))
    suite.addTests(unittest.TestLoader().loadTestsFromTestCase(TestInputValidation))

    # 运行测试
    runner = unittest.TextTestRunner(verbosity=2)
    result = runner.run(suite)

    # 打印总结
    print(f"\n{'='*60}")
    print(f"📊 合并测试总结:")
    print(f"   运行测试: {result.testsRun}")
    print(f"   成功: {result.testsRun - len(result.failures) - len(result.errors)}")
    print(f"   失败: {len(result.failures)}")
    print(f"   错误: {len(result.errors)}")
    print(f"{'='*60}")

    return result.wasSuccessful()


if __name__ == '__main__':
    success = run_all_tests()
    sys.exit(0 if success else 1)
