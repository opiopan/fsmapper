import clsx from 'clsx';
import Link from '@docusaurus/Link';
import useBaseUrl from '@docusaurus/useBaseUrl';
import useDocusaurusContext from '@docusaurus/useDocusaurusContext';
import Layout from '@theme/Layout';
import HomepageFeatures from '@site/src/components/HomepageFeatures';
import HomepageTopics from '@site/src/components/HomepageTopics';

import Heading from '@theme/Heading';
import styles from './index.module.scss';

import { Version } from '@site/.version';

function HomepageHeader() {
  const {siteConfig} = useDocusaurusContext();
  return (
    <header className={clsx('hero hero--primary', styles.heroBanner)}>
      <div className="container">
        <Heading as="h1" className={clsx('hero__title', styles.heroTitleText, styles.heroTitle)}>
          <img src={useBaseUrl('img/logo.svg')} className={styles.titleIcon} alt='fsmapper logo'/>
          {siteConfig.title}
        </Heading>
        <p className={clsx('hero__subtitle', styles.heroTitleText)}>{siteConfig.tagline}</p>
        <div className={clsx(styles.buttons)}>
          <Link
            className={clsx('button button--primary button--lg', styles.button, styles.buttonPrime)}
            to="/intro">
            Get Started
          </Link>
          <Link
            className={clsx('button button--secondary button--lg', styles.button)}
            to={Version.package}>
            Download {Version.text}
          </Link>
        </div>
      </div>
    </header>
  );
}

export default function Home() {
  const {siteConfig} = useDocusaurusContext();
  return (
    <Layout
      title={`${siteConfig.title} - ${siteConfig.tagline}`}
      description={`${siteConfig.tagline}`}>
      <HomepageHeader />
      <main>
        <HomepageFeatures />
        <HomepageTopics />
      </main>
    </Layout>
  );
}
