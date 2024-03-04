// @ts-check
// `@type` JSDoc annotations allow editor autocompletion and type checking
// (when paired with `@ts-check`).
// There are various equivalent ways to declare your Docusaurus config.
// See: https://docusaurus.io/docs/api/docusaurus-config

import {themes as prismThemes} from 'prism-react-renderer';
import {Version} from './.version.js';

/** @type {import('@docusaurus/types').Config} */
const config = {
  title: 'fsmapper User\'s Guide',
  tagline: 'Good relation between various devices and filght simurator',
  favicon: 'img/favicon.ico',

  // Set the production url of your site here
  url: 'https://opiopan.github.io/',
  // Set the /<baseUrl>/ pathname under which your site is served
  // For GitHub pages deployment, it is often '/<projectName>/'
  baseUrl: '/fsmapper/',

  // GitHub pages deployment config.
  // If you aren't using GitHub pages, you don't need these.
  organizationName: 'opiopan', // Usually your GitHub org/user name.
  projectName: 'fsmapper', // Usually your repo name.

  onBrokenLinks: 'throw',
  onBrokenMarkdownLinks: 'warn',

  // Even if you don't use internationalization, you can use this field to set
  // useful metadata like html lang. For example, if your site is Chinese, you
  // may want to replace "en" with "zh-Hans".
  i18n: {
    defaultLocale: 'en',
    locales: ['en'],
  },

  presets: [
    [
      'classic',
      /** @type {import('@docusaurus/preset-classic').Options} */
      ({
        docs: {
          routeBasePath: '/',
          sidebarPath: './sidebars.js',
        },
        blog: {
          showReadingTime: true,
        },
        theme: {
          customCss: './src/css/custom.css',
        },
      }),
    ],
  ],

  themeConfig:
    /** @type {import('@docusaurus/preset-classic').ThemeConfig} */
    ({
      navbar: {
        title: 'fsmapper User\'s Guide',
        logo: {
          alt: 'famapper logo',
          src: 'img/logo.svg',
        },
        hideOnScroll: false,
        items: [
          {
            href: Version.package,
            label: 'Download fsmapper',
            position: 'right',
          },
          {
            href: 'https://github.com/opiopan/fsmapper',
            label: 'GitHub',
            position: 'right',
          },
        ],
      },
      docs: {
        sidebar: {
          hideable: true,
        },
      },
      footer: {
        style: 'dark',
        links: [
          {
            title: 'Documentation',
            items: [
              {
                label: 'Introduction',
                to: '/',
              },
              {
                label: 'Configuration Guide',
                to: '/category/configuration-guide',
              },
              {
                label: 'Library Reference',
                to: '/libs/',
              },
            ],
          },
          {
            title: 'Contact',
            items: [
              {
                label: 'opiopan@gmail.com',
                href: 'mailto:opiopan@gmail.com',
              },
            ],
          },
          {
            title: 'More',
            items: [
              {
                label: 'GitHub',
                href: 'https://github.com/opiopan/fsmapper',
              },
            ],
          },
        ],
        copyright: `Copyright © ${new Date().getFullYear()} Hiroshi Murayama &lt;opiopan@gmail.com&gt;.`,
      },
      prism: {
        theme: prismThemes.github,
        darkTheme: prismThemes.dracula,
        additionalLanguages: ['lua'],
      },
    }),
};

export default config;
