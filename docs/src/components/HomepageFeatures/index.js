import clsx from 'clsx';
import Heading from '@theme/Heading';
import styles from './styles.module.css';
import useBaseUrl from '@docusaurus/useBaseUrl';

const FeatureList = [
  {
    title: 'Enhanced Touchscreen Cockpit',
    imagesrc: 'img/touchscreen.webp',
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
    title: 'Custom Input Mapping',
    imagesrc: 'img/axis_mapping.webp',
    description: (
      <>
        With fsmapper, you can fully customize the characteristics of your input devices, 
        such as flight sticks and throttles. 
        For example, you can adjust analog axis curves, or invert button states.
        This level of control ensures a highly personalized and precise simulation experience.
      </>
    ),
  },
  {
    title: 'Flexibility by Lua',
    imagesrc: 'img/lua.webp',
    description: (
      <>
        Manage all functions with Lua scripts, 
        offering comprehensive control and customization. 
        Create, reuse, and manage your configurations with ease using your preferred editor 
        and version control systems like git, eliminating the need for complex GUIs.
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
