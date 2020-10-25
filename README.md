# About

Nodejs cli that helps to create a new plugin for flextool. Bootstrap a new project from a template.

NOTE: Template contain filenames that may be invalid on Windows, so use tools like Docker under Windows.

## Usage

Install NodeJs and npm: https://nodejs.org/en/download/

```bash
# Tested with NodeJs v10.18.1
node -v

# TypeScript Installation
npm install -g typescript
```

Install requirements:

```bash
# `--no-audit` because no need in security audit for project generator.
npm install --no-audit
```

Build:

```bash
npm run build
```

Run:

```bash
node ./dist/index.js \
  --name my_plugin_name \
  --copyright my_plugin_copyright \
  --template external_plugin

# open generated folder based on `--name`
cd my_plugin_name
```

## Standing on the Shoulders of Giants

Project inspired by articles:
- https://dev.to/duwainevandriel/build-your-own-project-template-generator-59k4
- https://medium.com/@pongsatt/how-to-build-your-own-project-templates-using-node-cli-c976d3109129

## How add new cli options (like desired project name)

1. edit `TemplateData` in `src/utils/template.ts`
2. `QUESTIONS` and `CliOptions` in `src/index.ts`
3. Map `answers` to `CliOptions` like so:
   ```js
   const projectName = answers['name'];
   ```
4. Pass new option into `template.render` like so:
   ```js
   template.render(contents, { projectName });
   ```

## How to run template engine on desired files

Use in contents of desired files ejs template syntax i.e. `<%= projectName %>`

Use in filenames of desired files ejs template syntax i.e. `<%= projectName %>`
