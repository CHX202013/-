# Security Policy

马尾是内核级 root 工具，本身即高权限软件。使用前请确认：

- 设备是你自己的，或你已获得明确授权；
- 已备份 stock boot 镜像；
- 理解「隐藏」不等于「不可检测」，也不要用它绕过他人的设备管控。

## 报告问题

请通过仓库的 Private Security Advisory 提交，不要在公开 issue 里贴可利用细节。

## 已知边界

- LKM 模式会在 `/proc/modules` 留下模块项（见 `docs/stealth.md`）；
- SELinux domain 仍为 `u:r:mw:s0`；
- 本项目不提供「保证绕过某检测产品」的承诺。
