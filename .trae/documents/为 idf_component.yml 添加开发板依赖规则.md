## 任务概览
从 [Kconfig](file:///home/nix/ws/nix-wrapper-esp32/Kconfig) 中提取开发板配置选项，并在 [idf_component.yml](file:///home/nix/ws/nix-wrapper-esp32/idf_component.yml) 中为对应的开发板组件添加依赖规则（`rules`），确保组件仅在选中相应开发板时才被引入。

## 详细步骤
1. **分析 Kconfig 配置**：
   - 确认开发板配置项为 `NIX_WRAPPER_BOARD_M5STACK_CORES3` 和 `NIX_WRAPPER_BOARD_M5STACK_TAB5`。
   - 在 `idf_component.yml` 中引用时需使用前缀 `CONFIG_`。

2. **修改 idf_component.yml**：
   - 为 `espressif/m5stack_core_s3` 添加规则：
     ```yaml
     rules:
       - if: "CONFIG_NIX_WRAPPER_BOARD_M5STACK_CORES3 == y"
     ```
   - 为 `espressif/m5stack_tab5` 添加规则：
     ```yaml
     rules:
       - if: "CONFIG_NIX_WRAPPER_BOARD_M5STACK_TAB5 == y"
     ```

## 预期结果
- [idf_component.yml](file:///home/nix/ws/nix-wrapper-esp32/idf_component.yml) 将包含基于 Kconfig 选项的条件依赖，从而优化组件管理，避免在未选中的开发板上下载不必要的依赖项。

请确认以上方案，以便我开始执行。