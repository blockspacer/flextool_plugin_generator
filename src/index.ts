#!/usr/bin/env node

import * as inquirer from 'inquirer';
import * as fs from 'fs';
import * as path from 'path';
import * as shell from 'shelljs';
import * as template from './utils/template';
import chalk from 'chalk';
import * as yargs from 'yargs';

const CHOICES = fs.readdirSync(path.join(__dirname, 'templates'));

const QUESTIONS = [
  {
    name: 'template',
    type: 'list',
    message: 'What project template would you like to generate?',
    choices: CHOICES,
    when: () => !yargs.argv['template']
  },
  {
    name: 'name',
    type: 'input',
    message: 'Project name:',
    when: () => !yargs.argv['name'],
    validate: (input: string) => {
      if (/^([A-Za-z\-\_\d])+$/.test(input)) return true;
      else return 'Project name may only include letters, numbers, underscores and hashes.';
    }
  },
  {
    name: 'copyright',
    type: 'input',
    message: 'Project copyright:',
    when: () => !yargs.argv['copyright'],
    validate: (input: string) => {
      if (/^([A-Za-z\-\_\d])+$/.test(input)) return true;
      else return 'Project copyright may only include letters, numbers, underscores and hashes.';
    }
  }
];

const CURR_DIR = process.cwd();

export interface TemplateConfig {
  files?: string[]
  postMessage?: string
}

export interface CliOptions {
  projectName: string
  copyright: string
  templateName: string
  templatePath: string
  tartgetPath: string
  config: TemplateConfig
}

inquirer.prompt(QUESTIONS)
  .then(answers => {

    answers = Object.assign({}, answers, yargs.argv);

    const projectChoice = answers['template'];
    const projectName = answers['name'];
    const copyright = answers['copyright'];
    const templatePath = path.join(__dirname, 'templates', projectChoice);
    const tartgetPath = path.join(CURR_DIR, projectName);
    const templateConfig = getTemplateConfig(templatePath);

    const options: CliOptions = {
      projectName,
      copyright,
      templateName: projectChoice,
      templatePath,
      tartgetPath,
      config: templateConfig
    }

    if (!createProject(tartgetPath)) {
      return;
    }

    createDirectoryContents(
      templatePath
      , projectName
      , templateConfig
      , {
        projectName
        , copyright
      });

    if (!postProcess(options)) {
      return;
    }

    showMessage(options);
  });

function showMessage(options: CliOptions) {
  console.log('');
  console.log(chalk.green('Done.'));
  console.log(chalk.green(`Go into the project: cd ${options.projectName}`));

  const message = options.config.postMessage;

  if (message) {
    console.log('');
    console.log(chalk.yellow(message));
    console.log('');
  }

}

function getTemplateConfig(templatePath: string): TemplateConfig {
  const configPath = path.join(templatePath, '.template.json');

  if (!fs.existsSync(configPath)) return {};

  const templateConfigContent = fs.readFileSync(configPath);

  if (templateConfigContent) {
    return JSON.parse(templateConfigContent.toString());
  }

  return {};
}

function createProject(projectPath: string) {
  if (fs.existsSync(projectPath)) {
    console.log(chalk.red(`Folder ${projectPath} exists. Delete or use another name.`));
    return false;
  }

  fs.mkdirSync(projectPath);
  return true;
}

function postProcess(options: CliOptions) {
  if (isNode(options)) {
    return postProcessNode(options);
  }
  return true;
}

function isNode(options: CliOptions) {
  return fs.existsSync(path.join(options.templatePath, 'package.json'));
}

function postProcessNode(options: CliOptions) {
  shell.cd(options.tartgetPath);

  let cmd = '';

  if (shell.which('yarn')) {
    cmd = 'yarn';
  } else if (shell.which('npm')) {
    cmd = 'npm install';
  }

  if (cmd) {
    const result = shell.exec(cmd);

    if (result.code !== 0) {
      return false;
    }
  } else {
    console.log(chalk.red('No yarn or npm found. Cannot run installation.'));
  }

  return true;
}

const SKIP_FILES = [
  'node_modules',
  '.template.json',
  '.png',
  '.jpg',
  '.jpeg',
  '.bmp',
  '.gif',
  '.tiff',
  '.aif',
  '.cda',
  '.mid',
  '.midi',
  '.mp3',
  '.mpa',
  '.ogg',
  '.wav',
  '.wma',
  '.wpl',
  '.7z',
  '.arj',
  '.deb',
  '.pkg',
  '.rar',
  '.rpm',
  '.tar.gz',
  '.z',
  '.zip',
  '.bin',
  '.dmg',
  '.iso',
  '.toast',
  '.vcd',
  '.dat',
  '.db',
  '.dbf',
  '.log',
  '.tmp',
  '.directory',
  '.sav',
  '.sql',
  '.tar',
  '.email',
  '.eml',
  '.msg',
  '.apk',
  '.bat',
  '.cgi',
  '.pl',
  '.exe',
  '.dll',
  '.so',
  '.lib',
  '.a',
  '.jar',
  '.msi',
  '.fnt',
  '.fon',
  '.otf',
  '.ttf',
  '.ai',
  '.psd',
  '.ico',
  '.ps',
  '.svg',
  '.ppt',
  '.pptx',
  '.key',
  '.pps',
  '.odp',
  '.pdf',
  '.doc',
  '.docx'
];

function createDirectoryContents(
  templatePath: string
  , projectName: string
  , config: TemplateConfig
  , tplOptions: template.TemplateData)
{
  const filesToCreate = fs.readdirSync(templatePath);

  console.log(`processing files in directory ${templatePath}`);

  filesToCreate.forEach(fileName => {
    const origFilePath = path.join(templatePath, fileName);

    // get stats about the current file
    const stats = fs.statSync(origFilePath);

    for (const skipName in SKIP_FILES) {
      if (fileName.indexOf(SKIP_FILES[skipName]) > -1) {
        console.log(`skipped file name ${fileName} because it contains ${SKIP_FILES[skipName]}`);
        return;
      }
    }

    if (stats.isFile()) {
      let contents = fs.readFileSync(origFilePath, 'utf8');

      console.log(chalk.green(`Processing file name: ${origFilePath}`));

      const newFileName
        = template.render(fileName, tplOptions);

      console.log(chalk.green(`Output file name: ${newFileName}`));

      console.log(chalk.green(`Processing file contents for ${origFilePath}`));

      const newContents
        = template.render(contents, tplOptions);

      const writePath
        = path.join(CURR_DIR, projectName, newFileName);

      fs.writeFileSync(writePath, newContents, 'utf8');
    } else if (stats.isDirectory())
    {
      console.log(chalk.green(`Processing directory name: ${origFilePath}`));

      const newDirectoryName
        = template.render(fileName, tplOptions);

      console.log(chalk.green(`Output directory name: ${newDirectoryName}`));

      fs.mkdirSync(path.join(CURR_DIR, projectName, newDirectoryName));

      // recursive call
      createDirectoryContents(
        path.join(templatePath, fileName)
        , path.join(projectName, newDirectoryName)
        , config
        , tplOptions);
    }
  });
}
