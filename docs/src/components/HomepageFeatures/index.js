import clsx from 'clsx';
import Heading from '@theme/Heading';
import styles from './styles.module.css';
import useBaseUrl from '@docusaurus/useBaseUrl';

const FeatureList = [
  {
    title: 'Enhanced Touchscreen Cockpit',
    imagesrc: 'img/touchscreen.png',
    description: (
      <>
        Seamlessly integrate Microsoft Flight Simulator’s pop-out instruments and 
        your own custom-drawn instruments using touchscreen devices. 
        Easily switch between multiple panels on the screen, 
        maximizing the use of small displays for a realistic and intuitive cockpit experience.
      </>
    ),
  },
  {
    title: 'Versatile Home Cockpit Building',
    imagesrc: 'img/homemade.png',
    description: (
      <>
        Unlock the potential of home cockpit building with fsmapper.
        The fsmapper Plugin SDK empowers you to create custom plugins to support new protocols, 
        while seamlessly integrating with standard USB gaming devices such as flight sticks.
      </>
    ),
  },
  {
    title: 'Flexibility by Lua',
    imagesrc: 'img/lua.png',
    description: (
      <>
        Manage all functions with Lua scripts, 
        offering comprehensive control and customization. 
        Create, reuse, and manage your configurations with ease using your preferred editor 
        and version control systems like Git, eliminating the need for complex GUIs.
      </>
    ),
  },
];

function Feature({imagesrc, title, description}) {
  return (
    <div className={clsx('col col--4')}>
      <div className="text--center">
        <img className={styles.featureImg} src={useBaseUrl(imagesrc)}/>
      </div>
      <div className="text--center padding-horiz--md">
        <Heading as="h3">{title}</Heading>
        <p>{description}</p>
      </div>
    </div>
  );
}

export default function HomepageFeatures() {
  return (
    <section className={styles.features}>
      <div className="container">
        <div className="row">
          {FeatureList.map((props, idx) => (
            <Feature key={idx} {...props} />
          ))}
        </div>
      </div>
    </section>
  );
}
