namespace Wizard
{
    public interface IPluginInfo
    {
        /** @brief Return plugin name as string */
        public string GetName();
        /** @brief Return a description as string */
        public string GetDescription();
        /** @brief Return a URL as string */
        public string GetURL();
        /** @brief Return tag as string */
        public string GetTag();
        /** @brief Return author as string */
        public string GetAuthor();
        /** @brief Return license as string */
        public string GetLicence();
        /** @brief Return version as string */
        public string GetVersion();
        /** @brief Return data as string */
        public string GetDate();
    }
}
