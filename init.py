import os
import subprocess
import yaml

with open('thirdparty/Imagine_System/config/submodules.yaml', 'r') as file:
    config_data = yaml.safe_load(file)

# 获取所有子模块信息
submodules_info = config_data['submodules']

# 确保路径中所有的中间目录都存在
submodule_dir_path = 'thirdparty/Imagine_System/worker/'
os.makedirs(os.path.dirname(submodule_dir_path), exist_ok=True)

for submodule in submodules_info:
    # 构造完整的子模块路径
    submodule_path = os.path.join(submodule_dir_path, submodule['name'])

    # 判断子模块目录是否存在
    if os.path.isdir(submodule_path):
        # 子模块已存在，切换到指定commit
        checkout_command = ["git", "checkout", submodule['git_commit']]
        process = subprocess.Popen(checkout_command, cwd=submodule_path)
        process.wait()
        # with tools.chdir(submodule['name']):
        #     checkout_command = f"git checkout {submodule['git_commit']}"
        #     subprocess.call(checkout_command, shell=True)
    else:
        # 子模块未存在，克隆并切换到指定commit
        clone_command = f"git clone --branch {submodule['git_branch']} {submodule['git_url']} {submodule_path}"
        subprocess.call(clone_command, shell=True)
        checkout_command = ["git", "checkout", submodule['git_commit']]
        process = subprocess.Popen(checkout_command, cwd=submodule_path)
        process.wait()
        # with tools.chdir(submodule['name']):
        #     checkout_command = f"git checkout {submodule['git_commit']}"
        #     subprocess.call(checkout_command, shell=True)
