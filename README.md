# claude-plugins (private)

Private Claude Code plugin marketplace. Everything here is for the private
builds only. The skills and agents name private checkouts, proprietary library
paths, and the public-sync strip rules, so this repository must stay private
and nothing in it is ever copied to a public mirror.

## Install

```
/plugin marketplace add dewilkinson/claude-plugins
/plugin install splatlab-dev@dewilkinson
```

The marketplace is a private GitHub repo, so `gh auth` (or a git credential
helper with access to it) must be set up on the machine first.

To iterate on the plugin without reinstalling, load it straight from disk:

```
claude --plugin-dir C:\github\claude-plugins\plugins\splatlab-dev
```

## Plugins

### splatlab-dev

Five roles for the private SplatLab repo at `C:\github\surfels`. The first two
run in the main session because they need to talk to the user. The last three
are subagents that work from the documents the first two produce, so they never
need to ask anything.

| Role | Kind | Invoke | Reads | Writes |
|------|------|--------|-------|--------|
| requirements | skill | `/splatlab-dev:requirements <name>` | repo, docs | `docs/requirements/<slug>.md` |
| architect | skill | `/splatlab-dev:architect <slug>` | requirements doc, code | `docs/design/<slug>.md`, `docs/private/<slug>-design.md` |
| coder | agent | prompt with the design path | design + requirements | code, one commit on the current branch |
| tester | agent | prompt with checkout path and what to verify | design test plan | logs and screenshots in scratch, a pass/fail table |
| reviewer | agent | prompt with commit range and design path | diff, design, requirements | ranked findings |

The tester's `tools:` list includes the Playwright MCP server declared in the
private repo's `.mcp.json`. Claude Code refuses to launch a subagent whose tool
list names a tool that is not available, so the tester only launches inside
the private repo (or another repo that configures a `playwright` MCP server).
The plugin deliberately does not ship its own server.

Source of truth for these files is this repo. The same files also exist under
`C:\github\surfels\.claude\` from the commit that introduced them; when the
plugin is installed those copies are redundant and can be removed there, which
also keeps them out of the public sync.
