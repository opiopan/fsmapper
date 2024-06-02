import clsx from 'clsx';
import Link from '@docusaurus/Link';
import useBaseUrl from '@docusaurus/useBaseUrl';
import Heading from '@theme/Heading';
import {Version} from '@site/.version';
import styles from './styles.module.css';

const TopicList = [
    {
        title: 'Get Started with Smaple scripts',
        imagesrc: 'img/sample-execution.jpg',
        description: (
            <>
                <p>
                    No special installation process is required to use fsmapper.
                    Simply download <Link to={Version.package}> this zip archive </Link> and extract it
                    to your preferred location.
                    The touchscreen cockpit configuration is written in Lua. Sample scripts for
                    several aircraft are included, so try those first.
                    Refer to <Link to='/samples/'>this guide</Link> for instructions on how to use the sample scripts.
                </p>
                <p>
                    Additional virtual cockpit script examples for more aircraft can be 
                    found <Link to='https://github.com/opiopan/scripts_for_fsmapper'>here</Link>.
                </p>
                <p>
                    If you are creating scripts for other aircraft or customizing scripts for your own setup, 
                    refer to the <Link to='/getting-started/tutorial'>Tutrial</Link> and <Link to='/category/configuration-guide'>Configuration Guide</Link>.
                </p>
            </>
        ),
    },
    {
        title: 'Touchscreen Cockpit Showcase',
        videosrc: 'https://www.youtube.com/embed/rIp1M2r1_ko',
        description: (
            <>
                <p>
                    This video showcases customized cockpits for eight different aircraft as examples of 
                    touchscreen cockpits created with fsmapper. Except for the example of the DA40, 
                    which displays the Garmin G1000 on a touchscreen, 
                    you can see instrument panels seamlessly integrating custom touch-operated components 
                    and pop-out instrument windows from Microsoft Flight Simulator, 
                    using <code>[Right Alt]></code> + <code>[Left Click]</code>. 
                    The video demonstrates switching between and operating multiple such instrument panels.
                </p>
                <p>
                    Note that in this video, the iPad is used as a 10.5-inch secondary touchscreen of Windows
                    via <Link to='https://astropad.com/product/lunadisplay/'>Luna Display</Link>. Additionally,
                    a <Link to='https://github.com/opiopan/simhid-g1000'>custom-made G1000-style bezel device</Link> is
                    used as a physical input device.
                </p>
            </>
        ),
    },
];

function Video({src}) {
    return (
        <div className={clsx(styles.topicMovie, styles.topicImg)}>
            <iframe
                width="100%" height="100%"
                src={src}
                title="YouTube video player" frameborder="0"
                allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture; web-share"
                allowfullscreen="">
            </iframe>
        </div>
    );
}

function Topic({imagesrc, videosrc, title, description}) {
    return (
        <div className={clsx("row", styles.topicRow)}>
            <div className={clsx('col col--6', styles.topicCol)}>
                <div className={clsx('text-left')}>
                    <Heading as="h1">{title}</Heading>
                    {description}
                </div>
            </div>
            <div className={clsx('col col--6', styles.topicCol)}>
                {imagesrc ? 
                    <div className={clsx('text-center', styles.topicImgContainer)}>
                        <img className={styles.topicImg} src={useBaseUrl(imagesrc)} />
                    </div>
                : videosrc ?
                    <div className={clsx('text-center', styles.topicImgContainer)}>
                        <Video src={videosrc}/>
                    </div>
                :
                    <div/>
                }
            </div>
        </div>
    );
}

export default function HomepageTopics() {
    return (
        <div className={styles.topics}>
            {TopicList.map((props, idx) => (
                <section className={styles.topic}>
                    <div className="container">
                        <Topic key={idx} {...props} />
                    </div>
                </section>
            ))}
        </div>
    );
}
