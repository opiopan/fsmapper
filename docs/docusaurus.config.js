// @ts-check
// `@type` JSDoc annotations allow editor autocompletion and type checking
// (when paired with `@ts-check`).
// There are various equivalent ways to declare your Docusaurus config.
// See: https://docusaurus.io/docs/api/docusaurus-config

import {themes as prismThemes} from 'prism-react-renderer';
import {Version} from './.version.js';

/** @type {import('@docusaurus/types').Config} */
const config = {
  title: 'fsmapper',
  tagline: 'A Virtual Instrument Panel and Home Cockpit Builder for MSFS and DCS',
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

  markdown: {
    hooks: {
      onBrokenMarkdownLinks: 'warn',
    },
  },

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
          async sidebarItemsGenerator({ defaultSidebarItemsGenerator, ...args }) {
            const sidebarItems = await defaultSidebarItemsGenerator(args);
            return sidebarItems.filter(
              item => !(args.item.dirName == '.' && item.type == 'category' && item.label == 'Plugin SDK')
            );
          },
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
        title: 'fsmapper',
        logo: {
          alt: 'famapper logo',
          src: 'img/logo.svg',
        },
        hideOnScroll: false,
        items: [
          {
            label: 'User Guide',
            to: '/intro',
            position: 'left',
            activeBaseRegex: '^/fsmapper/(?!$|sdk)',
          },
          {
            type: 'docSidebar',
            sidebarId: 'sdkSidebar',
            docsPluginId: 'sdkddocs',
            position: 'left',
            label: 'Plugin SDK',
          },
          {
            href: Version.package,
            label: 'Download',
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
                to: '/intro',
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
            title: 'Project',
            items: [
              {
                label: 'GitHub',
                href: 'https://github.com/opiopan/fsmapper',
              },
            ],
          },
          {
            title: 'More for me',
            items: [
              {
                label: 'My GitHub profile',
                href: 'https://github.com/opiopan/',
              },
            ],
          },
        ],
        copyright: `Copyright © ${new Date().getFullYear()} Hiroshi Murayama &lt;opiopan@gmail.com&gt;.`,
      },
      prism: {
        theme: prismThemes.dracula,
        darkTheme: prismThemes.dracula,
        additionalLanguages: ['lua'],
      },
    }),

  plugins: [
    'docusaurus-plugin-sass', [
      require.resolve('docusaurus-lunr-search'), {
        languages: ['en', 'de'] // language codes
      }
    ],
    [
      '@docusaurus/plugin-content-docs',
      {
        id: 'sdkddocs',
        path: 'sdkdocs',
        routeBasePath: 'sdk',
        sidebarPath: require.resolve('./sidebars.js'),
      }
    ]
  ],
};

export default config;
