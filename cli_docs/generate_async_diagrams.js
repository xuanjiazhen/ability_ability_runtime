#!/usr/bin/env node

/**
 * 生成异步执行架构图
 *
 * 需要安装 mermaid-cli:
 * npm install -g @mermaid-js/mermaid-cli
 *
 * 使用方法:
 * node generate_async_diagrams.js
 */

const fs = require('fs');
const path = require('path');
const { execSync } = require('child_process');

const inputDir = path.join(__dirname, 'diagrams');
const inputFile = path.join(inputDir, 'async_architecture.mmd');
const outputFile = path.join(inputDir, 'async_architecture.png');

console.log('生成异步执行架构图...');

try {
  // 使用 mermaid-cli 生成图片
  const cmd = `mmdc -i ${inputFile} -o ${outputFile} -b transparent -s 2`;
  console.log(`执行命令: ${cmd}`);

  execSync(cmd, { stdio: 'inherit' });

  console.log(`✅ 架构图已生成: ${outputFile}`);
  console.log(`文件大小: ${fs.statSync(outputFile).size} bytes`);
} catch (error) {
  console.error('❌ 生成失败:', error.message);
  console.log('\n提示: 请先安装 mermaid-cli:');
  console.log('  npm install -g @mermaid-js/mermaid-cli');
  process.exit(1);
}
